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
#include "HashKey.hpp"

#include "type_ios.hpp"
#include "macros/default_exceptions.hpp"

namespace OpenEFW
{
	class Component : public UnknownClass
	{
		SetUnknownClass

	protected:

		using This = Component;
		using ParamID = string const&;
		using Key = HashKey;

		Key m_id;

		bool m_created = false;
		This* m_parent = nullptr;
		void setParent(This& c) { m_parent = &c; };

		template<typename ...> struct ComponentFunction;

		template<typename T, typename ...A>
		struct ComponentFunction<T(A...)>
		{ using Type = Delegate<T(This*, A...)>; };

	public:
		template<typename T> using Function = typename ComponentFunction<T>::Type;
		template<typename T> using Value = Container<T>;
		using BaseValue = Container<>;

		using Lists = SmartMap<Key, BaseValue, Key>;

	protected:
		Lists m_values;
		Lists m_functions;
		Lists m_components;
		
		#define CompID(x)  "[" + to_str() + "::" + x + "] "
		#define CompType CompID(id) + TypeInfo::Get<Value<T>>::to_str()
		#define CompFunc CompID("@static") + type_to_str<I,T>()

		template<typename T>
		T& replace(Lists &lists, ParamID id, ParamID old_id)
		{
			auto &value = get<T>(lists, id);
			if (old_id != id) get<T>(lists, old_id) = value; // old
			// / && !lists.replace(old_id, new Value<T>(value))) CompNoRpl(TypeInfo::Get<Value<T>>::str());  // 
			return value;
		}

		template<typename T, typename ...Args>
		T& add(Lists &lists, ParamID id, Args... args)
		{
			if (!lists.add(Key::create<T>(id), new Value<T>(forward<Args>(args)...))) ThrowNotAdd(CompType);
			return get<T>(lists, id);
		}

		template<typename T>
		bool del(Lists &lists, ParamID id)
		{
			if (!has<T>(lists, key)) ThrowNotExist(CompType);
			return lists.del(Key::create<T>(id));
		}

		template<typename T>
		T& get(Lists &lists, ParamID id)
		{
			if (!has<T>(lists, id)) ThrowNotExist(CompType);
			auto base = lists.get(Key::create<T>(id));
			return **reconvertValue<T>(id, base);
		};

		template<typename T>
		bool has(Lists &lists, ParamID id)
		{ return lists.has(Key::create<T>(id));	};

		template<typename T>
		Value<T>* reconvertValue(ParamID id, BaseValue* base)
		{
			if (!base) ThrowIsNull(CompType);
			auto value = base ? base->reconvert<Value<T>>() : nullptr; //base->cast<T>()
			if (!value) ThrowNotCast(CompType);
			return value;
		}

		template<typename I, typename T>
		static inline string type_to_str()
		{
			static auto str = TypeInfo::Get<I>::to_str() + "::"
				+ TypeInfo::Get<T>::to_str();
			return str;
		};

		template<typename I, typename T>
		static inline Delegate<T>& static_function()
		{ static Delegate<T> func; return func;	};

	public:

		void printValues()
		{
			printf("\nValues of %s\n", c_str());
			printf("--------------------\n");
			m_values.loop([&](const Lists::Id& id, const Lists::Type &obj) { printf("%s\n", id.to_str().c_str()); return false; });
			printf("--------------------\n");
		}

		void printFunctions()
		{
			printf("\nFunctions of %s\n", c_str());
			printf("--------------------\n");
			m_functions.loop([&](const Lists::Id& id, const Lists::Type &obj) { printf("%s\n", id.to_str().c_str()); return false; });
			printf("--------------------\n");
		}

		void printComponents()
		{
			printf("\nComponents of %s\n", c_str());
			printf("--------------------\n");
			m_components.loop([&](const Lists::Id& id, const Lists::Type &obj) { printf("%s\n", id.to_str().c_str()); return false; });
			printf("--------------------\n");
		}

		void setID(ParamID id) { m_id = Key(id); };

		size_t to_hash() const { return m_id.to_hash(); };
		string to_str() { auto str = m_id.to_str();  return str != "" ? str.c_str() : "(Unknown Component)"; }
		const char* c_str() { return to_str().c_str(); }

		This& parent() { return *m_parent; };

		Lists& values() { return m_values; };
		Lists& functions() { return m_functions; };
		Lists& components() { return m_components; };

		template<typename T>
		enable_if_t<is_constructible<T>::value && !is_same<This, typename decay<T>::type>::value, T&>
		replace(ParamID id, ParamID old_id)
		{ return replace<T>(m_values, id, old_id); }

		template<typename T>
		enable_if_t<is_constructible<T>::value && !is_same<This, typename decay<T>::type>::value, T&>
		replace(ParamID id)
		{ return replace<T>(m_values, id, id); }

		template<typename T, typename ...Args>
		enable_if_t<is_constructible<T, Args...>::value && !is_same<This, typename decay<T>::type>::value, T&>
		add(ParamID id, Args... args)
		{ return add<T>(m_values, id, forward<Args>(args)...); }

		template<typename T>
		enable_if_t<is_constructible<T>::value && !is_same<This, typename decay<T>::type>::value, T&>
		get(ParamID id)
		{ return get<T>(m_values, id); }

		template<typename T>
		enable_if_t<is_constructible<T>::value && !is_same<This, typename decay<T>::type>::value, bool>
		del(ParamID id)
		{ return del<T>(m_values, id); }

		template<typename T>
		enable_if_t<is_constructible<T>::value && !is_same<This, typename decay<T>::type>::value, bool>
		has(ParamID id)
		{ return has<T>(m_values, id); }

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Function<T>>::value, Function<T>&>
		replace(ParamID id, ParamID old_id)
		{ return replace<Function<T>>(m_functions, id, old_id); }

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Function<T>>::value, Function<T>&>
		replace(ParamID id)
		{ return replace<Function<T>>(m_functions, id, id); }

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Function<T>>::value, Function<T>&>
		add(ParamID id)
		{ return add<Function<T>>(m_functions, id); }

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Function<T>>::value, Function<T>&>
		get(ParamID id)
		{ return get<Function<T>>(m_functions, id); }

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Function<T>>::value, bool>
		del(ParamID id)
		{ return del<Function<T>>(m_functions, id); }

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Function<T>>::value, bool>
		has(ParamID id)
		{ return has<Function<T>>(m_functions, id); }

		template<typename T>
		enable_if_t<is_same<This, typename decay<T>::type>::value, This&>
		replace(ParamID id, ParamID old_id)
		{
			auto &c = replace<T>(m_components, id, old_id);
			c.setID(id);
			setParent(c);
			return c;
		}

		template<typename T>
		enable_if_t<is_same<This, typename decay<T>::type>::value, This&>
		replace(ParamID id)
		{
			auto &c = replace<T>(m_components, id, id);
			c.setID(id);
			setParent(c);
			return c;
		}

		template<typename T>
		enable_if_t<is_same<This, typename decay<T>::type>::value, This&>
		add(ParamID id)
		{
			auto &c = add<T>(m_components, id);
			c.setID(id);
			setParent(c);
			return c;
		}

		template<typename T>
		enable_if_t<is_same<This, typename decay<T>::type>::value, This&>
		get(ParamID id)
		{ return get<T>(m_components, id); }

		template<typename T>
		enable_if_t<is_same<This, typename decay<T>::type>::value, bool>
		del(ParamID id)
		{ return del<T>(m_components, id); }

		template<typename T>
		enable_if_t<is_same<This, typename decay<T>::type>::value, bool>
		has(ParamID id)
		{ return has<T>(m_components, id); }

		template<typename R = void, typename ...A>
		R call(ParamID id, A... args)
		{
			using T = R(A...);
			if (!has<T>(id)) ThrowNotExist(CompType);
			return get<T>(id)(this, forward<A>(args)...);
		};

		template<typename T>
		void invoke(ParamID id)
		{
			for (auto &e : m_components.source()) {
				auto &c = *e.second.get()->get<This>();
				if (c.has<T>()) c.call<T>(id);
			}
		}

		template<typename I, typename T>
		enable_if_t<is_class<I>::value && is_trivial<I>::value, Delegate<T>&>
		get()
		{
			auto& current = static_function<I, T>();
			if (!current) ThrowNotExist(CompFunc);
			return current;
		};

		template<typename I, typename T>
		enable_if_t<is_class<I>::value && is_trivial<I>::value, Delegate<T>&>
		add()
		{
			auto& current = static_function<I, T>();
			if (current) ThrowNotAdd(CompFunc);
			return current;
		};

		template<typename I, typename T>
		enable_if_t<is_class<I>::value && is_trivial<I>::value && !is_array<T>::value, bool> // TODO: replace is_array<T> with sth that checks functions
		add(T const& f)
		{
			using F = Delegate<T>::Type;
			auto& current = static_function<I, F>();
			bool result = !current;
			if (result) current = f;
			return result;
		};

		template<typename I, typename J = void, typename T>
		enable_if_t<is_class<I>::value && is_trivial<I>::value && !is_same<I, J>::value, Delegate<T>&>
		replace()
		{
			auto& func = static_function<I, T>();
			if (!func) ThrowNotExist(CompFunc);
			auto& oldfunc = static_function<J, T>();
			oldfunc = func;
			return func;
		};

		template<typename C, typename I, typename J = void, typename T>
		enable_if_t<is_class<I>::value && is_trivial<I>::value && !is_same<I, J>::value, void>
		replace(Delegate<T> f)
		{ replace<C, I, J, T>() = f; };

		// invoke static function
		template<typename I, typename R = void, typename ...A>
		enable_if_t<is_class<I>::value && is_trivial<I>::value, R>
		invoke(A... args)
		{
			using T = R(A...);
			auto f = static_function<I, T>();
			if (!f) ThrowNotExist(CompFunc);
			return (f)(forward<A>(args)...);
		};

		// call static function with current component object
		template<typename I, typename R = void, typename ...A>
		enable_if_t<is_class<I>::value && is_trivial<I>::value, R>
		call(A... args)
		{
			using T = R(This*,A...);
			auto f = static_function<I, T>();
			if (!f) ThrowNotExist(CompFunc);
			return (f)(this, forward<A>(args)...);
		};

		template<typename T>
		This& operator+=(const Arguments<string, T>& in)
		{ add<T>(in.a1) = in.a2; return *this; };

		template<typename T>
		This& operator=(const Arguments<string, T>& in)
		{ replace<T>(in.a1) = in.a2; return *this; };

		template<typename T>
		This& operator+=(const Arguments<const char*, T>& in)
		{ add<T>(string(in.a1)) = in.a2; return *this; };

		template<typename T>
		This& operator=(const Arguments<const char*, T>& in)
		{ replace<T>(string(in.a1)) = in.a2; return *this; };
	
		void operator()() { create(); };

		This& operator=(const This& other){ copy(const_cast<This&>(other)); return *this; };

		bool operator==(const This& other){ return m_id == other.m_id; };
		bool operator==(const decltype(m_id)& other){ return m_id == other; };
		bool operator==(const ParamID& other){ return m_id == Key(other); };
		bool operator!=(const This& other){ return !(*this == other); };
		bool operator!=(const decltype(m_id)& other){ return !(*this == other); };
		bool operator!=(const ParamID& other){ return !(*this == other); };

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
		explicit This(ParamID id) { setID(id); }; \
		explicit This(bool usepreset) { if (usepreset) preset(*this); }; \
		explicit This(ParamID id, bool usepreset) { setID(id); if (usepreset) preset(*this); };

		~Component()
		{
			if (m_created && has<void()>("@delete"))
				call("@delete");
		};

	protected:

		void copy(Lists& that, Lists& other)
		{
			that.clear();
			//other.loop([&](const Lists::Id& id, const Lists::Type &obj) { that.add(id, reconvertValue<T>(obj)); return false; });
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

		void copy(This& other)
		{
			m_id = other.m_id;
			if (!m_created) m_created = other.m_created;
			if (!m_parent) m_parent = other.m_parent;

			copy(m_values, other.m_values);
			copy(m_functions, other.m_functions);
			copy(m_components, other.m_components);

			if (m_created && has<void()>("@copy")) call("@copy");
		};

		void steal(This& other)
		{
			steal(m_values, other.m_values);
			steal(m_functions, other.m_functions);
			steal(m_components, other.m_components);

			if (m_created && has<void()>("@steal")) call("@steal");
		};

		void swap(This& other)
		{
			swap(m_values, other.m_values);
			swap(m_functions, other.m_functions);
			swap(m_components, other.m_components);

			if (m_created && has<void()>("@swap")) call("@swap");
		};

		void restart() { m_created = false; (*this)(); }

		void resetValues(){ m_values.clear(); }
		void resetFunctions(){ m_functions.clear(); }
		void resetComponents(){ m_components.clear(); }

		void reset()
		{
			resetValues();
			resetFunctions();
			resetComponents();
		}
	
		static void preset(Component& c)
		{
			if (!c.has<void()>("@create")) c.add<void()>("@create") = [](This*){};
			if (!c.has<void()>("@delete")) c.add<void()>("@delete") = [](This*){};
			if (!c.has<void()>("@copy")) c.add<void()>("@copy") = [](This*){};
			if (!c.has<void()>("@steal")) c.add<void()>("@steal") = [](This*){};
			if (!c.has<void()>("@swap")) c.add<void()>("@swap") = [](This*){};
		};
	};
};

#endif