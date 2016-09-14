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
	This class is an own invention of the delegate class (replacement for std::function)
	http://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11
	http://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates
	http://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible
*/
#pragma once
#ifndef __OPENEFW_DELEGATE_HPP__
#define __OPENEFW_DELEGATE_HPP__

#include "type_traits.hpp"
#include "type_functional.hpp"
#include "type_utility.hpp"
#include "type_memory.hpp"

#include "exception.hpp"
#include "hash_map.hpp"

#include "macros/file_x.hpp"

namespace OpenEFW
{
	template <typename ...T> class Delegate;

	template <typename T> struct Delegate<T> : public Delegate<decltype(&T::operator())> {};
	template <typename C, typename R, typename... A> struct Delegate<R(C::*)(A...) const> : public Delegate<R(A...)> {
		using ClassType = C;
		using FunctionType = R(C::*)(A...) const;
	};

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
		using stub_ptr_type = R(*)(void*, A...);
		using func_ptr_type = R(*)(A...);

	private:
		void default() { m_typeinfo.set<R(A...)>(); };

		Delegate(void* const o, stub_ptr_type const m) _NOEXCEPT :
		m_object_ptr(o), m_stub_ptr(m) { default(); }

	public:
		using ReturnType = R;
		using Type = R(A...);
		using This = Delegate<Type>;
		using Super = Delegate<>;

		Delegate() { default(); } // = default;

		Delegate(Delegate const&) = default;

		// This won't work (tested in MSVC)
		//Delegate(Delegate&&) = default; 
		//Delegate& operator=(Delegate&&) = default;

		Delegate(nullptr_t const) _NOEXCEPT : Delegate() {}

		template <class C, typename = typename enable_if< is_class<C>::value>::type>
		explicit Delegate(C const* const o) _NOEXCEPT : m_object_ptr(const_cast<C*>(o)) { default(); }

		template <class C, typename = typename enable_if< is_class<C>::value>::type>
		explicit Delegate(C const& o) _NOEXCEPT : m_object_ptr(const_cast<C*>(&o)) { default(); }

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
		m_store(operator new(sizeof(typename decay<T>::type)),
		functor_deleter<typename decay<T>::type>),
		m_store_size(sizeof(typename decay<T>::type))
		{
			using functor_type = typename decay<T>::type;
			default();

			new (m_store.get()) functor_type(forward<T>(f));

			m_object_ptr = m_store.get();
			m_stub_ptr = functor_stub<functor_type>;
			m_deleter = deleter_stub<functor_type>;
		}

		Delegate& operator=(Delegate const&) = default;

		template <class C>
		Delegate& operator=(R(C::* const rhs)(A...)){
			return *this = from(static_cast<C*>(m_object_ptr), rhs);
		}

		template <class C>
		Delegate& operator=(R(C::* const rhs)(A...) const) {
			return *this = from(static_cast<C const*>(m_object_ptr), rhs);
		}

		template <typename T, typename = typename enable_if<!is_same<Delegate, typename decay<T>::type>::value>::type>
		Delegate& operator=(T&& f)
		{
			using functor_type = typename decay<T>::type;

			if ((sizeof(functor_type) > m_store_size) || !m_store.unique())
			{
				m_store.reset(operator new(sizeof(functor_type)),
				functor_deleter<functor_type>);
				m_store_size = sizeof(functor_type);
			}
			else
			{
				m_deleter(m_store.get());
			}

			new (m_store.get()) functor_type(forward<T>(f));

			m_object_ptr = m_store.get();
			m_stub_ptr = functor_stub<functor_type>;
			m_deleter = deleter_stub<functor_type>;

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

		void reset() { if (!m_stub_ptr) return; m_stub_ptr = nullptr; m_store.reset(); }

		void reset_stub() _NOEXCEPT{ m_stub_ptr = nullptr; }

		void swap(Delegate& other) _NOEXCEPT{ swap(*this, other); }

		bool operator==(Delegate const& rhs) const _NOEXCEPT{
			return (m_object_ptr == rhs.m_object_ptr) && (m_stub_ptr == rhs.m_stub_ptr);
		}

		bool operator!=(Delegate const& rhs) const _NOEXCEPT{ return !operator==(rhs); }

		bool operator<(Delegate const& rhs) const _NOEXCEPT{
			return (m_object_ptr < rhs.m_object_ptr) ||
			((m_object_ptr == rhs.m_object_ptr) && (m_stub_ptr < rhs.m_stub_ptr));
		}

		bool operator==(nullptr_t const) const _NOEXCEPT { return m_stub_ptr == nullptr; }
		bool operator!=(nullptr_t const) const _NOEXCEPT { return m_stub_ptr != nullptr; }

		explicit operator bool() const _NOEXCEPT { return m_stub_ptr ? true : false; }

		R invoke(A... args) const {
			//assert(m_stub_ptr);
			if (!m_stub_ptr) throw Exception<This>("bad function call", __FILE__X, __LINE__);
			return m_stub_ptr(m_object_ptr, forward<A>(args)...);
		};

		R operator()(A... args) const {
			//assert(m_stub_ptr);
			if (!m_stub_ptr) throw Exception<This>("bad function call", __FILE__X, __LINE__);
			return m_stub_ptr(m_object_ptr, forward<A>(args)...);
		};

		stub_ptr_type target() const { return m_stub_ptr; };

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
		func_ptr_type m_func_ptr = nullptr;

	private:
		friend struct hash<Delegate>;

		using deleter_type = void(*)(void*);

		void* m_object_ptr = nullptr;
		stub_ptr_type m_stub_ptr = nullptr;

		deleter_type m_deleter = nullptr;

		shared_ptr<void> m_store;
		size_t m_store_size = 0;

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

		static decltype(m_stub_ptr) target(Super& ref) { return ref.target(); };
		static decltype(m_func_ptr) c_target(Super& ref) { return ref.c_target<This>(); };
	};
};

namespace std
{
	template <typename R, typename ...A>
	struct hash<::OpenEFW::Delegate<R(A...)> >
	{
		size_t operator()(::OpenEFW::Delegate<R(A...)> const& d) const _NOEXCEPT {
			auto const seed(hash<void*>()(d.m_object_ptr));
			return hash<typename ::OpenEFW::Delegate<R(A...)>::Type>()(d.m_stub_ptr) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};
};

#endif