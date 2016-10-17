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

#include <vector>

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

	public:
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
			static auto str = TypeInfo::Get<I>::to_str() + "::"	+ TypeInfo::Get<T>::to_str();
			return str;
		};

	public:
		~Component() { ~(*this); };

		enum class LIST { VALUES, FUNCTIONS, COMPONENTS, ALL };

		void reset(LIST id)
		{
			if (id == LIST::VALUES) { m_values.clear(); }
			else if (id == LIST::FUNCTIONS) { m_functions.clear(); }
			else if (id == LIST::COMPONENTS) { m_components.clear(); }
			else {
				reset(LIST::VALUES);
				reset(LIST::FUNCTIONS);
				reset(LIST::COMPONENTS);
			}
		}

		void print(ostream& ost, LIST id)
		{
			Lists* list = NULL;
			string name = "?";

			if (id == LIST::VALUES) { name="VALUES"; list = &m_values; }
			else if (id == LIST::FUNCTIONS) { name = "FUNCTIONS"; list = &m_functions; }
			else if (id == LIST::COMPONENTS) { name = "COMPONENTS"; list = &m_components; }
			else {
				print(ost, LIST::VALUES);
				print(ost, LIST::FUNCTIONS);
				print(ost, LIST::COMPONENTS);
				return;
			}

			if (list) {
				ost << "\n" << name << " of " << c_str() << "\n--------------------\n";
				list->loop([&](const Lists::Id& id, const Lists::Type &obj) { ost << id.to_str().c_str() << "\n"; return false; });
				ost << "--------------------\n";
			}
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

		template<typename T>
		enable_if_t<is_constructible<T>::value && !is_same<This, typename decay<T>::type>::value, T&>
		add(ParamID id)
		{ return add<T>(m_values, id); }

		template<typename T>
		enable_if_t<is_constructible<T>::value && !is_same<This, typename decay<T>::type>::value && !is_convertible<Delegate<>, T>::value, bool>
		add(ParamID id, T obj)
		{
			auto r = !has<T>(id);
			if (r) add<T>(m_values, id) = obj;
			return r;
		}

		template<typename T, typename ...Args>
		enable_if_t<is_constructible<T, Args...>::value && !is_same<This, typename decay<T>::type>::value, T&>
		add(ParamID id, Args... args)
		{ return add<T>(m_values, id, forward<Args>(args)...); }

		template<typename T>
		enable_if_t<is_constructible<T>::value && !is_same<This, typename decay<T>::type>::value, T&>
		get(ParamID id)
		{ return get<T>(m_values, id); }

		template<typename T>
		enable_if_t<is_constructible<T>::value && !is_same<This, typename decay<T>::type>::value && !is_convertible<Delegate<>, T>::value, bool>
		set(ParamID id, T obj)
		{
			auto r = has<T>(id);
			if (r) get<T>(m_values, id) = obj;
			return r;
		}

		template<typename T>
		enable_if_t<is_constructible<T>::value && !is_same<This, typename decay<T>::type>::value, bool>
		del(ParamID id)
		{ return del<T>(m_values, id); }

		template<typename T>
		enable_if_t<is_constructible<T>::value && !is_same<This, typename decay<T>::type>::value, bool>
		has(ParamID id)
		{ return has<T>(m_values, id); }

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Delegate<T>>::value, Delegate<T>&>
		replace(ParamID id, ParamID old_id)
		{ return replace<Delegate<T>>(m_functions, id, old_id); }

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Delegate<T>>::value, Delegate<T>&>
		replace(ParamID id)
		{ return replace<Delegate<T>>(m_functions, id, id); }

		template<typename T>
		enable_if_t<!is_constructible<T>::value && !is_array<T>::value, bool> // TODO: replace is_array<T> with sth that checks functions
		add(ParamID id, T const& f)
		{
			using C = Delegate<>::convert<T>::type;
			auto r = !has<C>(id);
			if (r) add<C>(m_functions, id) = f;
			return r;
		}

		template<typename T>
		enable_if_t<!is_constructible<T>::value && !is_array<T>::value && is_convertible<Delegate<>, Delegate<T>>::value, bool>
		add(ParamID id, Delegate<T> const& f)
		{
			using C = Delegate<T>;
			auto r = !has<C>(id);
			if (r) add<C>(m_functions, id) = f;
			return r;
		}

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Delegate<T>>::value, Delegate<T>&>
		add(ParamID id)
		{ return add<Delegate<T>>(m_functions, id); }

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Delegate<T>>::value, Delegate<T>&>
		get(ParamID id)
		{ return get<Delegate<T>>(m_functions, id); }

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Delegate<T>>::value, bool>
		del(ParamID id)
		{ return del<Delegate<T>>(m_functions, id); }

		template<typename T>
		enable_if_t<is_convertible<Delegate<>, Delegate<T>>::value, bool>
		has(ParamID id)
		{ return has<Delegate<T>>(m_functions, id); }

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

		// call with "this" parameter 
		template<typename R = void, typename ...A>
		R self(ParamID id, A... args)
		{
			using T = R(This*, A...);
			if (!has<T>(id)) ThrowNotExist(CompType);
			return get<T>(id)(this, forward<A>(args)...);
		};

		// call without "this" parameter 
		template<typename R = void, typename ...A>
		R call(ParamID id, A... args)
		{
			using T = R(A...);
			if (!has<T>(id)) ThrowNotExist(CompType);
			return get<T>(id)(forward<A>(args)...);
		};

		template<typename R = void, typename ...A>
		R invoke(ParamID comp_id, ParamID id, A... args)
		{
			return get<Component>(comp_id).call<R>(id, forward<A>(args)...);
		}

		// invoke a function per component with results
		template<typename R, typename ...A>
		void invokeAll(std::vector<R>& results, ParamID id, A... args)
		{
			using T = R(A...);
			for (auto &e : m_components.source()) {
				auto &c = reconvertValue<Component>("component_function('" + id + "')", e.second.get())->value; // could fail if not exists
				if (c.has<T>()) results.push_back(c.call<T>(id, forward<A>(args)...));
			}
		}

		// invoke a function per component without results
		template<typename R = void, typename ...A>
		void invokeAll(ParamID id, A... args)
		{
			using T = R(A...);
			for (auto &e : m_components.source()) {
				auto &c = reconvertValue<Component>("component_function('" + id + "')", e.second.get())->value; // could fail if not exists
				if (c.has<T>()) c.call<T>(id, forward<A>(args)...);
			}
		}

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
	
		void operator~() { destroy(); };
		void operator()() { create(); };

		This& operator=(const This& other){ copy(const_cast<This&>(other)); return *this; };

		bool operator==(const This& other){ return m_id == other.m_id; };
		bool operator==(const decltype(m_id)& other){ return m_id == other; };
		bool operator==(const ParamID& other){ return m_id == Key(other); };
		bool operator!=(const This& other){ return !(*this == other); };
		bool operator!=(const decltype(m_id)& other){ return !(*this == other); };
		bool operator!=(const ParamID& other){ return !(*this == other); };

		friend string operator+(const char* other, This& o) { return other + o.to_str(); };
		friend string operator+(string &other, This& o) { return other + o.to_str(); };
		friend void operator+=(string &other, This& o) { other += o.to_str(); };
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
			if (!m_created && has<void(This*)>("@create")) {
				m_created = true;
				self("@create");
			}
		};

		void destroy()
		{
			if (m_created && has<void(This*)>("@delete")) {
				self("@delete");
				m_created = false;
			}
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

			if (m_created && has<void(This*)>("@copy")) self("@copy");
		};

		void steal(This& other)
		{
			steal(m_values, other.m_values);
			steal(m_functions, other.m_functions);
			steal(m_components, other.m_components);

			if (m_created && has<void(This*)>("@steal")) self("@steal");
		};

		void swap(This& other)
		{
			swap(m_values, other.m_values);
			swap(m_functions, other.m_functions);
			swap(m_components, other.m_components);

			if (m_created && has<void(This*)>("@swap")) self("@swap");
		};

		void restart() { m_created = false; (*this)(); }

		static void preset(Component& c)
		{
			c.add("@create", [&](This*) {});
			c.add("@delete", [&](This*) {});
			c.add("@copy", [&](This*) {});
			c.add("@steal", [&](This*) {});
			c.add("@swap", [&](This*) {});
		};
	};
};

#endif