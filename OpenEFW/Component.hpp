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

		//template<typename T> Key key(ParamID id) { return Key::create<T>(id); }

		template<typename T>
		enable_if_t<!is_same<This, typename decay<T>::type>::value, void>
		update(ParamID id, T& c) {} // do nothing if not a component

		template<typename T>
		enable_if_t<is_same<This, typename decay<T>::type>::value, void>
		update(ParamID id, T& c) {
			c.setID(id);
			c.setParent(*this);
		}

		template<typename _T, typename ...Args, typename T = Delegate<>::convert<_T>::type>
		T& add(Lists &lists, Key k, Args... args)
		{
			if (!lists.add(k, new Value<T>(forward<Args>(args)...))) { auto& id = k.to_str(); ThrowNotAdd(CompType); }
			auto& value = get<T>(lists, k);
			update(k.to_str(), value);
			return value;
		}

		template<typename _T, typename T = Delegate<>::convert<_T>::type>
		T& get(Lists &lists, Key k)
		{
			auto& id = k.to_str();
			if (!lists.has(k)) ThrowNotExist(CompType);
			auto base = lists.get(k);
			return **reconvert<T>(id, base);
		};

		template<typename _T, typename T = Value<Delegate<>::convert<_T>::type>>
		T* reconvert(ParamID id, BaseValue* base)
		{
			if (!base) ThrowIsNull(CompType);
			auto value = base ? base->reconvert<T>() : nullptr; //base->cast<T>()
			if (!value) ThrowNotCast(CompType);
			return value;
		}

		template<typename I, typename T>
		static inline string type_to_str()
		{
			static auto str = TypeInfo::Get<I>::to_str() + "::"	+ TypeInfo::Get<T>::to_str();
			return str;
		};

		template<typename T>
		Lists& list()
		{
			Lists *l = NULL;

			using Type = Delegate<>::convert<T>::type;

			auto v = is_constructible<Type>::value;
			auto c = is_same<This, typename decay<Type>::type>::value;
			auto f = is_convertible<Delegate<>, Type>::value; //is_convertible<Delegate<>, Delegate<T>>::value || 

			v = v && !c && !f;
			c = c && !f && !v;
			f = f && !c && !v;

			if (v) l = &m_values;
			else if (c) l = &m_components;
			else if (f) l = &m_functions;
			else ThrowNotExist(string("List"));

			return *l;
		}

		template<typename T>
		void useOnLists(T f)
		{
			f(m_values);
			f(m_functions);
			f(m_components);
		}

		template<typename T>
		void useOnLists(This& other, T f)
		{
			f(m_values, other.m_values);
			f(m_functions, other.m_functions);
			f(m_components, other.m_components);
		}

	public:
		~Component() { ~(*this); };

		enum class LIST { VALUES, FUNCTIONS, COMPONENTS, ALL };

		Lists& getList(LIST id)
		{
			Lists* l = NULL;

			if (id == LIST::VALUES) l = &m_values;
			else if (id == LIST::FUNCTIONS) l = &m_functions;
			else if (id == LIST::COMPONENTS) l = &m_components;
			else ThrowNotExist(string("List"));

			return *l;
		};

		void reset(LIST id)
		{
			if (id == LIST::ALL) useOnLists([](Lists& l) { l.clear(); });
			else getList(id).clear();
		}

		void print(ostream& ost, LIST id)
		{
			auto f = [&](Lists& l)
			{
				string name = "?";

				if (id == LIST::VALUES) name = "VALUES";
				else if (id == LIST::FUNCTIONS) name = "FUNCTIONS";
				else if (id == LIST::COMPONENTS) name = "COMPONENTS";

				ost << "\n" << name << " of " << c_str() << "\n--------------------\n";
				l.loop([&](const Lists::Id& id, const Lists::Type &obj) { ost << id.to_str().c_str() << "\n"; return false; });
				ost << "--------------------\n";
			};

			if (id == LIST::ALL) useOnLists(f);
			else f(getList(id));
		}

		void setID(ParamID id) { m_id = Key(id); };

		size_t to_hash() const { return m_id.to_hash(); };
		string to_str() { auto str = m_id.to_str();  return str != "" ? str.c_str() : "(Unknown Component)"; }
		const char* c_str() { return to_str().c_str(); }

		This& parent() { return *m_parent; };

		// ------------------------------

		template<typename T>
		Key key(ParamID id)
		{
			return Key::create<Delegate<>::convert<T>::type>(id);
		}

		template<typename T>
		bool del(ParamID id)
		{
			auto& l = list<T>();
			auto& k = key<T>(id);
			if (!l.has(k)) ThrowNotExist(CompType);
			return l.del(k);
		}

		template<typename T>
		bool has(ParamID id) { return list<T>().has(key<T>(id)); };

		template<typename T>
		bool set(ParamID id, T const& obj)
		{
			auto& l = list<T>();
			auto& k = key<T>(id);
			auto r = l.has(k);
			if (r) { update(k.to_str(), obj); get<T>(l, k) = obj; }
			return r;
		}

		template<typename T, typename ...Args>
		bool add(ParamID id, T const& obj, Args... args)
		{
			auto& k = key<T>(id);
			auto& l = list<T>();
			auto r = !l.has(k);
			if (r) add<T>(l, k, forward<Args>(args)...) = obj;
			return r;
		}

		template<typename _T, typename ...Args, typename T = Delegate<>::convert<_T>::type>
		T& add(ParamID id, Args... args)
		{
			return add<T>(list<T>(), key<T>(id), forward<Args>(args)...);
		}

		template<typename _T, typename T = Delegate<>::convert<_T>::type>
		T& get(ParamID id)
		{
			return get<T>(list<T>(), key<T>(id));
		}

		template<typename T>
		bool replace(ParamID id, T& value)
		{
			return replace(id, id, value);
		};

		template<typename T>
		bool replace(ParamID id, ParamID old_id, T& newvalue)
		{
			bool rpl = (old_id != id);

			auto& l = list<T>();
			auto& k = key<T>(id);
			auto& k_ = key<T>(old_id);
			auto& value = l.has(k) ? get<T>(l, k) : add<T>(l, k);

			if (rpl) {
				auto &old = get<T>(l, k_);
				old = value;
				update(old_id, old);
			}

			value = newvalue;
			update(id, value);

			// / && !l.replace(old_id, new Value<T>(value))) CompNoRpl(TypeInfo::Get<Value<T>>::str());  // 
			return rpl;
		}

		// ------------------------------

		// call with "this" parameter 
		template<typename R = void, typename ...A>
		R self(ParamID id, A... args)
		{ return get<R(This*, A...)>(id)(this, forward<A>(args)...); };

		// call without "this" parameter 
		template<typename R = void, typename ...A>
		R call(ParamID id, A... args)
		{ return get<R(A...)>(id)(forward<A>(args)...); };

		template<typename R = void, typename ...A>
		R invoke(ParamID comp_id, ParamID id, A... args)
		{ return get<This>(comp_id).call<R>(id, forward<A>(args)...); }

		// invoke a function per component with results
		template<typename R, typename ...A>
		void invoke(std::vector<R>& results, ParamID id, A... args)
		{
			using T = R(A...);
			for (auto &e : m_components.source()) {
				auto &c = reconvert<This>("component_function('" + id + "')", e.second.get())->value; // could fail if not exists
				if (c.has<T>()) results.push_back(c.call<T>(id, forward<A>(args)...));
			}
		}

		// invoke a function per component without results
		template<typename R = void, typename ...A>
		void invoke(ParamID id, A... args)
		{
			using T = R(A...);
			for (auto &e : m_components.source()) {
				auto &c = reconvert<This>("component_function('" + id + "')", e.second.get())->value; // could fail if not exists
				if (c.has<T>()) c.callT>(id, forward<A>(args)...);
			}
		}

		template<typename T>
		This& operator+=(const Arguments<string, T>& in)
		{ add(in.a1, in.a2); return *this; };

		template<typename T>
		This& operator+=(const Arguments<const char*, T>& in)
		{ add(string(in.a1), in.a2); return *this; };

		template<typename T>
		This& operator=(const Arguments<string, T>& in)
		{ replace(in.a1, in.a2); return *this; };

		template<typename T>
		This& operator=(const Arguments<const char*, T>& in)
		{ replace(string(in.a1), in.a2); return *this; };
	
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

			useOnLists(other, [&](Lists& that, Lists& other)
			{
				that.clear();
				//other.loop([&](const Lists::Id& id, const Lists::Type &obj) { that.add(id, reconvert<T>(obj)); return false; });
				for (auto &e : other.source()) that.add(e.first, e.second.get());
			});

			if (m_created && has<void(This*)>("@copy")) self("@copy");
		};

		void steal(This& other)
		{
			useOnLists(other, [&](Lists& that, Lists& other)
			{
				that.clear();
				that = other;
				other.setRemover([&](const Lists::Id& id, const Lists::Type &obj) {}); // this object does not own objects anymore => no deletion
				other.clear();
				other.defaultRemover();
			});

			if (m_created && has<void(This*)>("@steal")) self("@steal");
		};

		void swap(This& other)
		{
			useOnLists(other, [&](Lists& that, Lists& other)
			{
				auto saved = that;

				that.setRemover([&](const Lists::Id& id, const Lists::Type &obj) {});
				that = other;
				that.defaultRemover();

				other.setRemover([&](const Lists::Id& id, const Lists::Type &obj) {});
				other = saved;
				other.defaultRemover();

				saved.setRemover([&](const Lists::Id& id, const Lists::Type &obj) {});
			});

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