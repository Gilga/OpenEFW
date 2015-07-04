/*
 * Copyright (c) 2015, Mario Link
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * * Neither the name of OpenEFW nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
Replacement for std::function, because its FASTER!
http://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11
http://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates
http://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible
*/
#pragma once
#ifndef __OPENEFW_DELEGATE_HPP__
#define __OPENEFW_DELEGATE_HPP__

#include <memory>

#include "exception.hpp"
#include "typeinfo.hpp"
#include "hash_map.hpp"

#include "macros/file_x.hpp"

namespace OpenEFW
{
	using ::std::true_type;
	using ::std::false_type;
	using ::std::nullptr_t;
	using ::std::enable_if;
	using ::std::enable_if_t;
	using ::std::is_class;
	using ::std::is_same;
	using ::std::decay;
	using ::std::forward;
	using ::std::pair;
	using ::std::swap;
	using ::std::hash;
	using ::std::shared_ptr;
	using ::std::is_convertible;

	template <typename ...T> class Delegate;

	template<> class Delegate<> {
	protected:
		using This = Delegate<>;
		
		TypeInfo m_typeinfo;
	
		virtual void default() = 0;

	public:
		virtual ~Delegate() {};

		TypeInfo getTypeInfo() { return m_typeinfo; };

		template<typename T> Delegate<T>* get()
		{
			if (m_typeinfo.hasType<T>())  return static_cast<Delegate<T>*>(this);
			return nullptr;
		}
	};

	template<typename R, typename ...A>
	class Delegate<R(A...)> : public Delegate<>
	{
	protected:
		using This = Delegate<R(A...)>;
		using Super = Delegate<>;
		using stub_ptr_type = R(*)(void*, A...);
		using func_ptr_type = R(*)(A...);

	private:
		void default() { m_typeinfo.set<R(A...)>(); };

		Delegate(void* const o, stub_ptr_type const m) _NOEXCEPT :
		object_ptr_(o), stub_ptr_(m) { default(); }

	public:
		Delegate() { default(); } // = default;

		Delegate(Delegate const&) = default;

		//Delegate(Delegate&&) = default;

		Delegate(nullptr_t const) _NOEXCEPT : Delegate() {}

		template <class C, typename = typename enable_if< is_class<C>::value>::type>
		explicit Delegate(C const* const o) _NOEXCEPT : object_ptr_(const_cast<C*>(o)) { default(); }

		template <class C, typename = typename enable_if< is_class<C>::value>::type>
		explicit Delegate(C const& o) _NOEXCEPT : object_ptr_(const_cast<C*>(&o)) { default(); }

		template <class C>
		Delegate(C* const object_ptr, R(C::* const method_ptr)(A...)) {
			default();
			*this = from(object_ptr, method_ptr);
		}

		template <class C>
		Delegate(C* const object_ptr, R(C::* const method_ptr)(A...) const) {
			default();
			*this = from(object_ptr, method_ptr);
		}

		template <class C>
		Delegate(C& object, R(C::* const method_ptr)(A...)) {
			default();
			*this = from(object, method_ptr);
		}

		template <class C>
		Delegate(C const& object, R(C::* const method_ptr)(A...) const) {
			default();
			*this = from(object, method_ptr);
		}
		
		template <typename T, typename = typename enable_if<!is_same<Delegate, typename decay<T>::type>::value>::type>
		Delegate(T&& f) :
		store_(operator new(sizeof(typename decay<T>::type)),
		functor_deleter<typename decay<T>::type>),
		store_size_(sizeof(typename decay<T>::type))
		{
			using functor_type = typename decay<T>::type;
			default();

			new (store_.get()) functor_type(forward<T>(f));

			object_ptr_ = store_.get();
			stub_ptr_ = functor_stub<functor_type>;
			deleter_ = deleter_stub<functor_type>;
		}

		Delegate& operator=(Delegate const&) = default;

		//Delegate& operator=(Delegate&&) = default;

		template <class C>
		Delegate& operator=(R(C::* const rhs)(A...)){
			return *this = from(static_cast<C*>(object_ptr_), rhs);
		}

		template <class C>
		Delegate& operator=(R(C::* const rhs)(A...) const) {
			return *this = from(static_cast<C const*>(object_ptr_), rhs);
		}

		template <typename T, typename = typename enable_if<!is_same<Delegate, typename decay<T>::type>::value>::type>
		Delegate& operator=(T&& f)
		{
			using functor_type = typename decay<T>::type;

			if ((sizeof(functor_type) > store_size_) || !store_.unique())
			{
				store_.reset(operator new(sizeof(functor_type)),
				functor_deleter<functor_type>);
				store_size_ = sizeof(functor_type);
			}
			else
			{
				deleter_(store_.get());
			}

			new (store_.get()) functor_type(forward<T>(f));

			object_ptr_ = store_.get();
			stub_ptr_ = functor_stub<functor_type>;
			deleter_ = deleter_stub<functor_type>;

			return *this;
		}

		template <R(*const function_ptr)(A...)>
		static Delegate from() _NOEXCEPT{
			return{ nullptr, function_stub<function_ptr> };
		}

		template <class C, R(C::* const method_ptr)(A...)>
		static Delegate from(C* const object_ptr) _NOEXCEPT{
			return{ object_ptr, method_stub<C, method_ptr> };
		}

		template <class C, R(C::* const method_ptr)(A...) const>
		static Delegate from(C const* const object_ptr) _NOEXCEPT{
			return{ const_cast<C*>(object_ptr), const_method_stub<C, method_ptr> };
		}

		template <class C, R(C::* const method_ptr)(A...)>
		static Delegate from(C& object) _NOEXCEPT{
			return{ &object, method_stub<C, method_ptr> };
		}

		template <class C, R(C::* const method_ptr)(A...) const>
		static Delegate from(C const& object) _NOEXCEPT{
			return{ const_cast<C*>(&object), const_method_stub<C, method_ptr> };
		}

		template <typename T>
		static Delegate from(T&& f) { return forward<T>(f); }

		static Delegate from(R(*const function_ptr)(A...)) { return function_ptr; }

		template <class C>
		using member_pair = pair<C* const, R(C::* const)(A...)>;

		template <class C>
		using const_member_pair = pair<C const* const, R(C::* const)(A...) const>;

		template <class C>
		static Delegate from(C* const object_ptr, R(C::* const method_ptr)(A...)) {
			return member_pair<C>(object_ptr, method_ptr);
		}

		template <class C>
		static Delegate from(C const* const object_ptr, R(C::* const method_ptr)(A...) const) {
			return const_member_pair<C>(object_ptr, method_ptr);
		}

		template <class C>
		static Delegate from(C& object, R(C::* const method_ptr)(A...)) {
			return member_pair<C>(&object, method_ptr);
		}

		template <class C>
		static Delegate from(C const& object, R(C::* const method_ptr)(A...) const) {
			return const_member_pair<C>(&object, method_ptr);
		}

		void reset() { if (!stub_ptr_) return; stub_ptr_ = nullptr; store_.reset(); }

		void reset_stub() _NOEXCEPT{ stub_ptr_ = nullptr; }

		void swap(Delegate& other) _NOEXCEPT{ swap(*this, other); }

		bool operator==(Delegate const& rhs) const _NOEXCEPT{
			return (object_ptr_ == rhs.object_ptr_) && (stub_ptr_ == rhs.stub_ptr_);
		}

		bool operator!=(Delegate const& rhs) const _NOEXCEPT{ return !operator==(rhs); }

		bool operator<(Delegate const& rhs) const _NOEXCEPT{
			return (object_ptr_ < rhs.object_ptr_) ||
			((object_ptr_ == rhs.object_ptr_) && (stub_ptr_ < rhs.stub_ptr_));
		}

		bool operator==(nullptr_t const) const _NOEXCEPT { return stub_ptr_ == nullptr; }
		bool operator!=(nullptr_t const) const _NOEXCEPT { return stub_ptr_ != nullptr; }

		explicit operator bool() const _NOEXCEPT { return stub_ptr_ ? true : false; }

		R invoke(A... args) const {
			//assert(stub_ptr_);
			if (!stub_ptr_) throw Exception<This>("bad function call", __FILE__X, __LINE__);
			return stub_ptr_(object_ptr_, forward<A>(args)...);
		};

		R operator()(A... args) const {
			//assert(stub_ptr_);
			if (!stub_ptr_) throw Exception<This>("bad function call", __FILE__X, __LINE__);
			return stub_ptr_(object_ptr_, forward<A>(args)...);
		};

		stub_ptr_type target() const { return stub_ptr_; };

		template<class C> func_ptr_type c_target(bool reset = false) { return callback<C>(this, reset); };

		// should be used only once per class! you can reset the entry of instance.
		template<class C> static func_ptr_type callback(This* target = nullptr, bool reset = false) {
			static This storrage;

			if (target && storrage && !reset) {
				throw Exception<This>("instance of c target already exist", __FILE__X, __LINE__);
			}
			else if (target && (reset || !storrage)) storrage = *target;
			else if (reset && !target && storrage) storrage.reset();

			return func_ptr_type([](A... args){ return (storrage)(forward<A>(args)...); });
		};

	protected:
		func_ptr_type func_ptr_ = nullptr;

	private:
		friend struct hash<Delegate>;

		using deleter_type = void(*)(void*);

		void* object_ptr_ = nullptr;
		stub_ptr_type stub_ptr_ = nullptr;

		deleter_type deleter_ = nullptr;

		shared_ptr<void> store_;
		size_t store_size_ = 0;

		template <class T>
		static void functor_deleter(void* const p)
		{
			static_cast<T*>(p)->~T();

			operator delete(p);
		}

		template <class T>
		static void deleter_stub(void* const p)
		{
			static_cast<T*>(p)->~T();
		}

		template <R(*function_ptr)(A...)>
		static R function_stub(void* const, A... args)
		{
			return function_ptr(forward<A>(args)...);
		}

		template <class C, R(C::*method_ptr)(A...)>
		static R method_stub(void* const object_ptr, A... args)
		{
			return (static_cast<C*>(object_ptr)->*method_ptr)(
				forward<A>(args)...);
		}

		template <class C, R(C::*method_ptr)(A...) const>
		static R const_method_stub(void* const object_ptr, A... args)
		{
			return (static_cast<C const*>(object_ptr)->*method_ptr)(
				forward<A>(args)...);
		}

		template <typename>
		struct is_member_pair : false_type { };

		template <class C>
		struct is_member_pair<pair<C* const,R(C::* const)(A...)> > : true_type {};

		template <typename>
		struct is_const_member_pair : false_type { };

		template <class C>
		struct is_const_member_pair<pair<C const* const, R(C::* const)(A...) const> > : true_type {};

		template <typename T>
		static enable_if_t<!(is_member_pair<T>::value || is_const_member_pair<T>::value), R>
		functor_stub(void* const object_ptr, A... args)
		{
			return (*static_cast<T*>(object_ptr))(forward<A>(args)...);
		}

		template <typename T>
		static enable_if_t<is_member_pair<T>::value || is_const_member_pair<T>::value, R>
		functor_stub(void* const object_ptr, A... args)
		{
			return (static_cast<T*>(object_ptr)->first->*
				static_cast<T*>(object_ptr)->second)(forward<A>(args)...);
		}
	};

	template<class C, typename T>
	class Delegate<C, T> : public Delegate<T>{
	protected:
		using This = Delegate<C, T>;
		using Super = Delegate<T>;
		using Base = Delegate<>;

		using Identifer = string;
		using Element = Delegate<T>;
		using Map = hash_map<Identifer, Element>;

		static Map& map() { static Map map; return map; };

	public:
		template<typename F> static void set(F ptr) { set(ptr, map().size()); };

		template<typename F> static void set(F ptr, size_t index) {
			set(to_string(index), ptr);
			//map()[to_string(index)] = ptr;
		};

		template<typename F> static void set(Identifer id, F ptr) {
			auto &it = map().find(id);
			if (it != map().end()) it->second = ptr;
			else map().insert(pair<Identifer, Element>(id, ptr));
			//map()[id] = ptr;
		};

		static Element& get(size_t index = 0) { return get(to_string(index)); };

		static Element& get(Identifer id) {
			auto &it = map().find(id);
			if (it != map().end()) return it->second;
			static Element default;
			return default;
		};

		static decltype(stub_ptr_) target(Super& ref) { return ref.target(); };
		static decltype(func_ptr_) c_target(Super& ref) { return ref.c_target<This>(); };
	};
};

namespace std
{
	template <typename R, typename ...A>
	struct hash<::OpenEFW::Delegate<R(A...)> >
	{
		size_t operator()(::OpenEFW::Delegate<R(A...)> const& d) const _NOEXCEPT {
			auto const seed(hash<void*>()(d.object_ptr_));
			return hash<typename ::OpenEFW::Delegate<R(A...)>::Type>()(d.stub_ptr_) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};
};

#endif