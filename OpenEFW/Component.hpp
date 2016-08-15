/*
 * Copyright (c) 2016, Mario Link
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

#include "Container.hpp"
#include "SmartMap.hpp"
#include "Arguments.hpp"

#include "type_ios.hpp"
#include "type_traits.hpp"
#include "type_functional.hpp"

#ifndef CompArg
#define CompArg(x,y) ::OpenEFW::Arguments<::OpenEFW::string, decltype(y)>{x,y};
#endif

namespace OpenEFW
{
	class Component : public UnknownClass
	{
		SetUnknownClass

	protected:

		using This = Component;
		using Identifer = string;

		struct Key : public binary_function<Key, Key, bool>
		{
		private:
			size_t id;
			Identifer content;

		public:

			template<typename T>
			static Key create(Identifer id) { return Key(id + "_" + TypeInfo::Get<T>::str()); }

			Key() = default;
			Key(Identifer c) : id(hash<decltype(c)>()(c)), content(c) {};

			size_t hashID() const { return id; };
			string to_str() const { return content; };

			void copy(const Key &other) { id = other.id; content = other.content; };

			static bool compare(const Key& x, const Key& y) { return x == y; };

			Key& operator=(const Key &other) { copy(other); return *this; };
			bool operator==(const Key &other) const { return (hashID() == other.hashID()); };
			bool operator<(const Key &other) const { return (hashID() < other.hashID()); };
			bool operator()(const Key& lhs, const Key& rhs) const { return lhs == rhs; };
			size_t operator()(const Key& k) const { return hashID(); };
		};

		size_t m_id = 0;
		Identifer m_name;

		bool m_created = false;
		This* m_parent = nullptr;
		void setParent(This& c) { m_parent = &c; };

		template<typename ...> struct ComponentFunction;
		template<typename T, typename ...A> struct ComponentFunction<T(A...)> { using Type = Delegate<T(This*, A...)>; };

	public:
		template<typename T> using Function = typename ComponentFunction<T>::Type;
		template<typename T> using Value = Container<T>;
		
		using Lists = SmartMap<Key, Container<>, Key>;

	protected:
		Lists m_values;
		Lists m_functions;
		Lists m_components;
		
		#define CompFuncNoExist(type) THROW_EXCEPTION(This, "'[" + m_name + "_" + id + "] " + type + "' static function does not exists.");
		#define CompFuncNoAdd(type) THROW_EXCEPTION(This, "'[" + m_name + "_" + id + "] " + type + "' static function could not be added.");

		#define CompNoExist(type) THROW_EXCEPTION(This, "'[" + m_name + "_" + id + "] " + TypeInfo::Get<type>::str() + "' does not exists.");
		#define CompNull(type) THROW_EXCEPTION(This, "'[" + m_name + "_" + id + "] " + TypeInfo::Get<type>::str() + "' is NULL.");
		#define CompNoCreate(type) THROW_EXCEPTION(This, "'[" + m_name + "_" + id + "] " + TypeInfo::Get<type>::str() + "' could not be created.");
		#define CompNoAdd(type) THROW_EXCEPTION(This, "'[" + m_name + "_" + id + "] " + TypeInfo::Get<type>::str() + "' could not be added. Entry already exists. Use replace command instead.");
		#define CompNoCast(type) THROW_EXCEPTION(This, "'[" + m_name + "_" + id + "] " +  TypeInfo::Get<type>::str() + "' could not be casted.");
		#define CompNoRpl(type) THROW_EXCEPTION(This, "'[" + m_name + "_" + id + "] " +  TypeInfo::Get<type>::str() + "' could not be replaced.");

		#define CreateKey() Key::create<T>(id)

		template<typename T> T& replace(Lists &lists, Identifer id, Identifer old_id) {
			auto &value = get<T>(lists, id);
			if (old_id != id) get<T>(lists, old_id) = value; // old
			// / && !lists.replace(old_id, new Value<T>(value))) CompNoRpl(TypeInfo::Get<Value<T>>::str());  // 
			return value;
		}

		template<typename T, typename ...Args> T& add(Lists &lists, Identifer id, Args... args) {
			//std::cout << "Add: " << CreateKey().to_str() << "\n";
			if (!lists.add(CreateKey(), new Value<T>(forward<Args>(args)...))) CompNoAdd(Value<T>);
			return get<T>(lists, id);
		}

		template<typename T> bool del(Lists &lists, Identifer id) {
			if (!has<T>(lists, key)) CompNoExist(Value<T>);
			return lists.del(CreateKey());
		}

		template<typename T> T& get(Lists &lists, Identifer id) {
			//std::cout << "Get: " << CreateKey().to_str() << "\n";
			using Type = Value<T>;
			if (!has<T>(lists, id)) CompNoExist(Type);
			auto base = lists.get(CreateKey());
			if (!base) CompNull(Type);
			auto container = base ? base->cast<T>() : nullptr; //base->reconvert<Type>()
			if (!container) CompNoCast(Type);
			return **container;
		};

		template<typename T> bool has(Lists &lists, Identifer id) {
			return lists.has(CreateKey());
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

	public:

		void printValues() {
			printf("\nValues of %s\n", m_name != "" ? m_name.c_str() : "?");
			printf("--------------------\n");
			m_values.loop([&](const Lists::Id& id, const Lists::Type &obj) { printf("%s\n", id.to_str().c_str()); return false; });
			printf("--------------------\n");
		}

		void printFunctions() {
			printf("\nFunctions of %s\n", m_name != "" ? m_name.c_str() : "?");
			printf("--------------------\n");
			m_functions.loop([&](const Lists::Id& id, const Lists::Type &obj) { printf("%s\n", id.to_str().c_str()); return false; });
			printf("--------------------\n");
		}

		void printComponents() {
			printf("\nComponents of %s\n", m_name != "" ? m_name.c_str() : "?");
			printf("--------------------\n");
			m_components.loop([&](const Lists::Id& id, const Lists::Type &obj) { printf("%s\n", id.to_str().c_str()); return false; });
			printf("--------------------\n");
		}

		Identifer name() { return m_name; };
		size_t id() { return m_id; };

		void setID(Identifer id) {
			m_name = id;
			m_id = hash<Identifer>()(id);
		};

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

		template<typename T> CompTVal(T&) replace(Identifer id, Identifer old_id) { return replace<T>(m_values, id, old_id); }
		template<typename T> CompTVal(T&) replace(Identifer id) { return replace<T>(m_values, id, id); }
		template<typename T, typename ...Args> CompTVal(T&) add(Identifer id, Args... args) { return add<T>(m_values, id, forward<Args>(args)...); }
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
			c.setID(id);
			setParent(c);
			return c;
		}

		template<typename T> CompTComp(This&) replace(Identifer id) {
			auto &c = replace<T>(m_components, id, id);
			c.setID(id);
			setParent(c);
			return c;
		}

		template<typename T> CompTComp(This&) add(Identifer id) {
			auto &c = add<T>(m_components, id);
			c.setID(id);
			setParent(c);
			return c;
		}

		template<typename T> CompTComp(This&) get(Identifer id) { return get<T>(m_components, id); }
		template<typename T> CompTComp(bool) del(Identifer id) { return del<T>(m_components, id); }
		template<typename T> CompTComp(bool) has(Identifer id) { return has<T>(m_components, id); }

		template<typename R = void, typename ...A> R call(Identifer id, A... args){
			if (!has<R(A...)>(id)) CompNoExist(R(A...));
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
			if (!current) CompFuncNoExist(Func::geType<T>());
			return current;
		};

		template<typename C, typename I, typename T> CompStaticTFunc(Function<T>&) add() {
			using Func = Static<C>;
			static Identifer id = "@static";
			auto& current = Func::function<I, T>();
			if (current) CompFuncNoAdd(Func::geType<T>());
			return current;
		};

		template<typename C, typename I, typename T> CompStaticTFunc(void) add(Function<T> f) { add<C, I, T>() = f; };

		template<typename C, typename I, typename J = void, typename T> CompStaticTFuncRpl(Function<T>&) replace() {
			using Func = Static<C>;
			static Identifer id = "@static";
			auto& func = Func::function<I, T>();
			if (!func) CompFuncNoExist(Static<I>::geType<T>());
			auto& oldfunc = Func::function<J, T>();
			oldfunc = func;
			return func;
		};

		template<typename C, typename I, typename J = void, typename T> CompStaticTFuncRpl(void) replace(Function<T> f) { replace<C, I, J, T>() = f; };

		template<typename C, typename I, typename R = void, typename ...A> CompStaticTCall(R) call(A... args) {
			using Func = Static<C>;
			static Identifer id = "@static";
			auto f = Func::function<I, R(A...)>();
			if (!f) CompFuncNoExist(Func::geType<R(A...)>());
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
		x() { setID(#x); }; \
		x(const x& other) { copy(const_cast<x&>(other)); }; \
		explicit x(bool usepreset) { setID(#x); if (usepreset) preset(*this); };

		This() = default; \
		This(const This& other) { copy(const_cast<This&>(other)); }; \
		explicit This(Identifer id) { setID(id); }; \
		explicit This(bool usepreset) { if (usepreset) preset(*this); }; \
		explicit This(Identifer id, bool usepreset) { setID(id); if (usepreset) preset(*this); };

		~Component() { if (m_created && has<void()>("@delete")) call("@delete"); };

		string str() { return to_string(m_id); }
		const char* c_str() { static string s; s = str(); return s.c_str(); }

	protected:

		void copy(Lists& that, Lists& other)
		{
			that.clear();
			for (auto &e : other.source()) that.add(e.first, e.second.get());
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
		};

	public:

		void copy(This& other){
			m_id = other.m_id;
			m_name = other.m_name;
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