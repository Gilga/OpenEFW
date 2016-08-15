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
#pragma once
#ifndef __OPENEFW_COMPONENT_HPP__
#define __OPENEFW_COMPONENT_HPP__

#include "Collection.hpp"
#include "SmartMap.hpp"
#include "Arguments.hpp"

#include "type_ios.hpp"
#include "type_traits.hpp"
#include "type_functional.hpp"

#ifndef CompArg
#define CompArg(x,y) ::OpenEFW::Arguments<::OpenEFW::string, decltype(y)>{x,y};
#endif

namespace OpenEFW {
	class Component : public UnknownClass {
		SetUnknownClass

	protected:

		using This = Component;
		using Identifer = string;

		struct Key : public binary_function<Key, Key, bool>
		{
			Identifer id;

			Key() = default;
			Key(Identifer id) : id(id) {};

			static bool compare(const Key& x, const Key& y) { return x == y; };
			static string toStr(Key& k) { return k.id; };

			Key& operator=(const Key &other) { id = other.id; return *this; };
			bool operator==(const Key &other) const	{ return (id == other.id); };
			bool operator<(const Key &other) const	{ return (id.length() < other.id.length()); };
			size_t operator()(const Key& k) const { return hash<decltype(k.id)>()(k.id); };
			bool operator()(const Key& lhs, const Key& rhs) const { return lhs == rhs; };
		};

		size_t m_id = 0;
		bool m_created = false;
		This* m_parent = nullptr;
		void setParent(This& c) { m_parent = &c; };

		template<typename ...> struct ComponentFunction;
		template<typename T, typename ...A> struct ComponentFunction<T(A...)> { using Type = Delegate<T(This*, A...)>; };

	public:
		template<typename T> using Function = typename ComponentFunction<T>::Type;
		template<typename T> using List = Collection<Key, T, Key, unordered_map<Key, T, Key>>;
		
		using Lists = SmartMap<size_t, Collection<>>;

	protected:
		Lists m_values;
		Lists m_functions;
		Lists m_components;

		template<typename T> T& replace(Lists &lists, Identifer id, Identifer old_id) {
			auto &key = Key(id);
			auto mlist = list<T>(id, lists, true);

			auto &value = mlist->has(key) ? mlist->get(key, true) : mlist->add(key, true);
			if (old_id != id) add<T>(lists, old_id) = value; // old

			return value;
		}

		template<typename T> T& add(Lists &lists, Identifer id) {
			return list<T>(id, lists, true)->add(Key(id));
		}

		template<typename T> bool del(Lists &lists, Identifer id) {
			return list<T>(id, lists, false)->del(Key(id));
		}
		template<typename T> T& get(Lists &lists, Identifer id) {
			return list<T>(id, lists, false)->get(Key(id));
		};

		template<typename T> bool has(Lists &lists, Identifer id) {
			auto mlist = list<T>(id, lists, false, true);
			return mlist ? mlist->has(Key(id)) : false;
		};

		template<typename T> using BUILD = is_constructible<T>;
		template<typename T> using CAST = is_convertible<Delegate<>, Function<T>>;
		template<typename T> using SAME = is_same<This, typename decay<T>::type>;

		template<typename I> struct Static
		{
			template<typename T> static inline string geType() {
				static auto str = TypeInfo::Get<I>::str() + "::" + TypeInfo::Get<T>::str(); return str;
			};

			template<typename I, typename T> static inline Function<T>& function() {
				static Function<T> func;
				return func;
			};
		};

		#define CompNoExist(type) THROW_EXCEPTION(This, "'[" + *this + "] [" + id + "] " + type + "' does not exists");
		#define CompNoCreate(type) THROW_EXCEPTION(This, "'[" + *this + "] [" + id + "] " + type + "' could not be created");
		#define CompNoAdd(type) THROW_EXCEPTION(This, "'[" + *this + "] [" + id + "] " + type + "' could not be added");
		#define CompNoCast(type) THROW_EXCEPTION(This, "'[" + *this + "] [" + id + "] " +  type + "' could not be casted");

		template<typename T> List<T>* list(const Identifer& id, Lists &lists, bool create, bool donoexcept = false){
			static auto hash_code = TypeInfo::Get<T>::hash_code();
			bool result = lists.has(hash_code);

			if (!result && create) {
				if (!lists.create<List<T>>(hash_code)) {
					if (donoexcept) return nullptr;
					CompNoCreate(TypeInfo::Get<List<T>>::str());
				}
				result = true;
			}

			if (!result) {
				if (donoexcept) return nullptr;
				CompNoExist(TypeInfo::Get<List<T>>::str());
			}

			auto mlist = lists.get(hash_code);
			auto nlist = mlist ? mlist->reconvert<List<T>>() : nullptr;
			if (!nlist){
				if (donoexcept) return nullptr;
				CompNoCast(TypeInfo::Get<List<T>>::str());
			}

			return nlist;
		}

	public:

		size_t& id() { return m_id; };
		void setId(Identifer id) { m_id = hash<Identifer>()(id); };

		This& parent() { return *m_parent; };

		Lists& values() { return m_values; };
		Lists& functions() { return m_functions; };
		Lists& components() { return m_components; };

		#define CompTVal(x) enable_if_t<BUILD<T>::value && !SAME<T>::value, x>
		#define CompTFunc(x) enable_if_t<CAST<T>::value, x>
		#define CompTComp(x) enable_if_t<SAME<T>::value, x>

		#define CompStaticTFunc(x) enable_if_t<is_class<I>::value && is_trivial<I>::value && CAST<T>::value, x>
		#define CompStaticTFuncRpl(x) enable_if_t<is_class<I>::value && is_trivial<I>::value && CAST<T>::value && !is_same<I, J>::value, x>
		#define CompStaticTCall(x) enable_if_t<is_class<I>::value && is_trivial<I>::value, x>

		template<typename T> enable_if_t<BUILD<T>::value && !SAME<T>::value, List<T>>& list(){ return *list<T>("?", m_values, false, false); }
		template<typename T> enable_if_t<CAST<T>::value, List<Function<T>>>&  list(){ return *list<Function<T>>("?", m_functions, false, false); }
		template<typename T> enable_if_t<SAME<T>::value, List<This>>& list(){ return *list<T>("?", m_components, false, false); }

		template<typename T> CompTVal(T&) replace(Identifer id, Identifer old_id) { return replace<T>(m_values, id, old_id); }
		template<typename T> CompTVal(T&) replace(Identifer id) { return replace<T>(m_values, id, id); }
		template<typename T> CompTVal(T&) add(Identifer id) { return add<T>(m_values, id); }
		template<typename T> CompTVal(T&) get(Identifer id) { return get<T>(m_values, id); }
		template<typename T> CompTVal(bool) del(Identifer id) { return del<T>(m_values, id); }
		template<typename T> CompTVal(bool) has(Identifer id) { return has<T>(m_values, id); }

		template<typename T> CompTFunc(Function<T>&) replace(Identifer id, Identifer old_id) { return replace<Function<T>>(m_functions, id, old_id); }
		template<typename T> CompTFunc(Function<T>&) replace(Identifer id) { return replace<Function<T>>(m_functions, id, id); }
		template<typename T> CompTFunc(Function<T>&) add(Identifer id) { return add<Function<T>>(m_functions, id); }
		template<typename T> CompTFunc(Function<T>&) get(Identifer id) { return get<Function<T>>(m_functions, id); }
		template<typename T> CompTFunc(bool) del(Identifer id) { return del<Function<T>>(m_functions, id); }
		template<typename T> CompTFunc(bool) has(Identifer id) { return has<Function<T>>(m_functions, id); }

		template<typename T> CompTComp(This&) replace(Identifer id, Identifer old_id) {
			auto &c = replace<T>(m_components, id, old_id);
			c.setId(id);
			setParent(c);
			return c;
		}

		template<typename T> CompTComp(This&) replace(Identifer id) {
			auto &c = replace<T>(m_components, id, id);
			c.setId(id);
			setParent(c);
			return c;
		}

		template<typename T> CompTComp(This&) add(Identifer id) {
			auto &c = add<T>(m_components, id);
			c.setId(id);
			setParent(c);
			return c;
		}

		template<typename T> CompTComp(This&) get(Identifer id) { return get<T>(m_components, id); }
		template<typename T> CompTComp(bool) del(Identifer id) { return del<T>(m_components, id); }
		template<typename T> CompTComp(bool) has(Identifer id) { return has<T>(m_components, id); }

		template<typename R = void, typename ...A> R call(Identifer id, A... args){
			if (!has<R(A...)>(id)) CompNoExist(TypeInfo::Get<R(A...)>::str());
			return get<R(A...)>(id)(this, forward<A>(args)...);
		};

		template<typename T> void invoke(Identifer id)
		{
			for (auto &e : m_components.source()) {
				auto &c = *e.second.get()->get<This>();
				if (c.has<T>()) c.call<T>(id);
			}
		}

		template<typename C, typename I, typename T> CompStaticTFunc(Function<T>&) get() {
			using Func = Static<C>;
			static Identifer id = "@static";
			auto& current = Func::function<I, T>();
			if (!current) CompNoExist(Func::geType<T>());
			return current;
		};

		template<typename C, typename I, typename T> CompStaticTFunc(Function<T>&) add() {
			using Func = Static<C>;
			static Identifer id = "@static";
			auto& current = Func::function<I, T>();
			if (current) CompNoAdd(Func::geType<T>());
			return current;
		};

		template<typename C, typename I, typename T> CompStaticTFunc(void) add(Function<T> f) { add<C, I, T>() = f; };

		template<typename C, typename I, typename J = void, typename T> CompStaticTFuncRpl(Function<T>&) replace() {
			using Func = Static<C>;
			static Identifer id = "@static";
			auto& func = Func::function<I, T>();
			if (!func) CompNoExist(Static<I>::geType<T>());
			auto& oldfunc = Func::function<J, T>();
			oldfunc = func;
			return func;
		};

		template<typename C, typename I, typename J = void, typename T> CompStaticTFuncRpl(void) replace(Function<T> f) { replace<C, I, J, T>() = f; };

		template<typename C, typename I, typename R = void, typename ...A> CompStaticTCall(R) call(A... args) {
			using Func = Static<C>;
			static Identifer id = "@static";
			auto f = Func::function<I, R(A...)>();
			if (!f) CompNoExist(Func::geType<R(A...)>());
			return (f)(this, forward<A>(args)...);
		};

		template<typename T> This& operator+=(const Arguments<string, T>& in) { add<T>(in.a1) = in.a2; return *this; };
		template<typename T> This& operator=(const Arguments<string, T>& in) { replace<T>(in.a1) = in.a2; return *this; };
	
		This& operator=(const This& other){ copy(const_cast<This&>(other)); return *this; };

		bool operator==(const This& other){ return m_id == other.m_id; };
		bool operator==(const decltype(m_id)& other){ return m_id == other; };
		bool operator==(const Identifer& other){ return m_id == hash<Identifer>()(other); };
		bool operator!=(const This& other){ return !(*this == other); };
		bool operator!=(const decltype(m_id)& other){ return !(*this == other); };
		bool operator!=(const Identifer& other){ return !(*this == other); };

		void operator()() { create(); };

		friend string operator+(const char* other, This& o) { return other + o.str(); };
		friend string operator+(string &other, This& o) { return other + o.str(); };
		friend void operator+=(string &other, This& o) { other += o.str(); };
		friend ostream& operator<<(ostream &other, This& o) { return other << "Component [" + o + "]"; };

		#define NewComponent(x) \
		SetUnknownClass\
		public: using This = x; \
		x() { setId(#x); }; \
		x(const x& other) { copy(const_cast<x&>(other)); }; \
		explicit x(bool usepreset) { setId(#x); if (usepreset) preset(*this); };

		This() = default; \
		This(const This& other) { copy(const_cast<This&>(other)); }; \
		explicit This(Identifer id) { setId(id); }; \
		explicit This(bool usepreset) { if (usepreset) preset(*this); }; \
		explicit This(Identifer id, bool usepreset) { setId(id); if (usepreset) preset(*this); };

		~Component() { if (m_created && has<void()>("@delete")) call("@delete"); };

		string str() { return to_string(m_id); }
		const char* c_str() { static string s; s = str(); return s.c_str(); }

	protected:

		void copy(Lists& that, Lists& other)
		{
			that.clear();
			//other.loop([&](Lists::Id id, Lists::Type type){ that.add(id, type->copy()); return false; });
			for (auto &e : other.source()) that.add(e.first, e.second.get()->copy());
		}

		void steal(Lists& that, Lists& other)
		{
			that.clear();
			that = other;
			other.setRemover([&](const Lists::Id& id, const Lists::Type &obj){}); // this object does not own objects anymore => no deletion
			other.clear();
			other.defaultRemover();
		}

		void swap(Lists& that, Lists& other)
		{
			auto saved = that;

			that.setRemover([&](const Lists::Id& id, const Lists::Type &obj){});
			that = other;
			that.defaultRemover();

			other.setRemover([&](const Lists::Id& id, const Lists::Type &obj){});
			other = saved;
			other.defaultRemover();

			saved.setRemover([&](const Lists::Id& id, const Lists::Type &obj){});
		}

		void create()
		{
			if (!m_created) m_created = true;

			if (has<void()>("@create")) call("@create");

			auto list = m_components.getByIndex(0);
			auto components = list ? list->reconvert<List<This>>() : nullptr;
			if (components) for (auto &e : components->map) e.second();
		};

	public:

		void copy(This& other){
			m_id = other.m_id;
			if (!m_created) m_created = other.m_created;
			if (!m_parent) m_parent = other.m_parent;

			copy(m_values, other.m_values);
			copy(m_functions, other.m_functions);
			copy(m_components, other.m_components);

			if (m_created && has<void()>("@copy")) call("@copy");
		};

		void steal(This& other){
			steal(m_values, other.m_values);
			steal(m_functions, other.m_functions);
			steal(m_components, other.m_components);

			if (m_created && has<void()>("@steal")) call("@steal");
		};

		void swap(This& other){
			swap(m_values, other.m_values);
			swap(m_functions, other.m_functions);
			swap(m_components, other.m_components);

			if (m_created && has<void()>("@swap")) call("@swap");
		};

		void restart() { m_created = false; (*this)(); }

		void resetValues(){ m_values.clear(); }
		void resetFunctions(){ m_functions.clear(); }
		void resetComponents(){ m_components.clear(); }

		void reset() {
			resetValues();
			resetFunctions();
			resetComponents();
		}
	
		static void preset(Component& c) {
			if (!c.has<void()>("@create")) c.add<void()>("@create") = [](This*){};
			if (!c.has<void()>("@delete")) c.add<void()>("@delete") = [](This*){};
			if (!c.has<void()>("@copy")) c.add<void()>("@copy") = [](This*){};
			if (!c.has<void()>("@steal")) c.add<void()>("@steal") = [](This*){};
			if (!c.has<void()>("@swap")) c.add<void()>("@swap") = [](This*){};
		};
	};
};

#endif