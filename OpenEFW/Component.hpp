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

#include <ostream>

#include "Collection.hpp"
#include "Delegate.hpp"
#include "SmartMap.hpp"

namespace OpenEFW {
	using ::std::ostream;
	using ::std::binary_function;
	using ::std::is_constructible;
	using ::std::is_trivial;
	using ::std::is_abstract;

	struct Component {
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

	protected:
		bool m_created = false;
		bool m_predefined = false;
		Component* m_parent = nullptr;
		void setParent(Component& c) { m_parent = &c; };

		template<typename ...> struct ComponentFunction;
		template<typename T, typename ...A> struct ComponentFunction<T(A...)> { using Type = Delegate<T(This*, A...)>; };

	public:
		template<typename T> using Function = typename ComponentFunction<T>::Type;
		template<typename T> using Components = Collection<Key, T, Key, unordered_map<Key, T, Key>>;
		
		using List = SmartMap<size_t, Collection<>>;

	protected:
		List m_list;
		Key createKey(Identifer id) { return{ id }; };

		static Identifer& tmp() { static Identifer id; return id; }

		template<typename T> T& _replace(Identifer id, Identifer old_id) {
			tmp() = id;
			auto &key = createKey(id);
			auto &klist = list<T>(true);

			auto &value = klist.has(key) ? klist.get(key, true) : klist.add(key, true);
			if (old_id != id) add<T>(old_id) = value; // old

			return value;
		}

		template<typename T> T& _add(Identifer id) { tmp() = id; return list<T>(true).add(createKey(id)); }
		template<typename T> bool _del(Identifer id) { tmp() = id; return list<T>().del(createKey(id)); }
		template<typename T> T& _get(Identifer id) { tmp() = id; return list<T>().get(createKey(id)); };

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

		template<typename T> struct In {
			Identifer id;
			T value;
			In(const Identifer &id, const T &value) : id(id), value(value) {};
		};

	public:

		template<typename T> Components<T>& list(bool create = false){
			static auto hash_code = TypeInfo::Get<T>::hash_code();
			bool result = m_list.has(hash_code);
			if (!result && create) {
				if (!m_list.create<Components<T>>(hash_code))
					THROW_EXCEPTION(This, "'" + tmp() + "' as " + TypeInfo::Get<T>::str() + " could not be created");
				result = true;
			}
			if (!result) THROW_EXCEPTION(This, "'" + tmp() + "' as " + TypeInfo::Get<T>::str() + " does not exists");
			auto mlist = m_list.get(hash_code);
			auto nlist = mlist ? mlist->get<Components<T>>() : nullptr;
			if (!nlist){
				THROW_EXCEPTION(This, "'" + tmp() + "' as " + TypeInfo::Get<T>::str() + " weird error");
			}
			return *nlist;
		}

		template<typename T> enable_if_t<BUILD<T>::value && !SAME<T>::value, T>& replace(Identifer id, Identifer old_id) { return _replace<T>(id, old_id); }
		template<typename T> enable_if_t<BUILD<T>::value && !SAME<T>::value, T>& replace(Identifer id) { return _replace<T>(id, id); }
		template<typename T> enable_if_t<BUILD<T>::value && !SAME<T>::value, T>& add(Identifer id) { return _add<T>(id); }
		template<typename T> enable_if_t<BUILD<T>::value && !SAME<T>::value, bool> del(Identifer id) { return _del<T>(id); }
		template<typename T> enable_if_t<BUILD<T>::value && !SAME<T>::value, T>& get(Identifer id) { return _get<T>(id); }

		template<typename T> enable_if_t<CAST<T>::value, Function<T>>& replace(Identifer id, Identifer old_id) { return _replace<Function<T>>(id, old_id); }
		template<typename T> enable_if_t<CAST<T>::value, Function<T>>& replace(Identifer id) { return _replace<Function<T>>(id, id); }
		template<typename T> enable_if_t<CAST<T>::value, Function<T>>& add(Identifer id) { return _add<Function<T>>(id); }
		template<typename T> enable_if_t<CAST<T>::value, bool> del(Identifer id) { return _del<Function<T>>(id); }
		template<typename T> enable_if_t<CAST<T>::value, Function<T>>& get(Identifer id) { return _get<Function<T>>(id); }
		
		template<typename T> enable_if_t<SAME<T>::value, This>& replace(Identifer id, Identifer old_id) { auto &c = _replace<T>(id, old_id); setParent(c); return c; }
		template<typename T> enable_if_t<SAME<T>::value, This>& replace(Identifer id) { auto &c = _replace<T>(id, id); setParent(c); return c; }
		template<typename T> enable_if_t<SAME<T>::value, This>& add(Identifer id) { auto &c = _add<T>(id); setParent(c); return c; }
		template<typename T> enable_if_t<SAME<T>::value, bool> del(Identifer id) { return _del<T>(id); }
		template<typename T> enable_if_t<SAME<T>::value, This>& get(Identifer id) { return _get<T>(id); }

		template<typename I, typename T>
		enable_if_t<is_class<I>::value && is_trivial<I>::value && CAST<T>::value, Function<T>>&
		get() {
			auto& current = Static<I>::function<I, T>();
			if (!current) THROW_EXCEPTION(This, Static<I>::geType<T>() + " function does not exists");
			return current;
		};

		template<typename I, typename T>
		enable_if_t<is_class<I>::value && is_trivial<I>::value && CAST<T>::value, Function<T>>&
		add() {
			auto& current = Static<I>::function<I, T>();
			if (current) THROW_EXCEPTION(This, Static<I>::geType<T>() + " function already exists");
			return current;
		};

		template<typename I, typename J = void, typename T>
		enable_if_t<is_class<I>::value && is_trivial<I>::value && CAST<T>::value && !is_same<I, J>::value, Function<T>>&
		replace() {
			auto& func = Static<I>::function<I, T>();
			if (!func) THROW_EXCEPTION(This, Static<I>::geType<T>() + " function does not exists");
			auto& oldfunc = Static<I>::function<J, T>();
			oldfunc = func;
			return func;
		};

		template<typename I, typename R = void, typename ...A>
		enable_if_t<is_class<I>::value && is_trivial<I>::value, R>
		call(A... args){
			if (!m_created) THROW_EXCEPTION(This, "can not call function " + Static<I>::geType<R(A...)>() + " when object is not created");
			auto f = Static<I>::function<I, R(A...)>();
			if (!f) THROW_EXCEPTION(This, Static<I>::geType<R(A...)>() + " function does not exists");
			return (f)(this, forward<A>(args)...);
		};

		template<typename R = void, typename ...A> R call(Identifer id, A... args){
			if (!m_created) THROW_EXCEPTION(This, "can not call function '" + id + "' as " + TypeInfo::Get<R(A...)>::str() + " when object is not created");
			return get<R(A...)>(id)(this, forward<A>(args)...);
		};

		template<typename T> This& operator=(const In<Function<T>> in){ replace<T>(in.id) = in.value; return *this; };
		template<typename T> This& operator=(const In<T> in){ replace<T>(in.id) = in.value; return *this; };

		This& operator=(const In<This>& in){ replace<This>(in.id) = in.value; return *this; };
		This& operator=(const This& other){ copy(const_cast<This&>(other)); return *this; };

		void operator()() { if (!m_created) { create(); if (m_predefined) call("@create"); } };

		friend ostream & operator<<(ostream &os, This& o) {
			if (o.m_predefined) return os << "Component '" << o.get<string>("@name") << "'";
			else return os << "Component";
		};

		void copy(This& other){
			auto &map = other.m_list;

			m_list.clear();
			map.loop([&](List::Id id, List::Type type){ m_list.add(id, type->copy()); return false; });
			//for (auto &e : map.source()) m_list.add(e.first, e.second.get()->copy());

			if (!m_created) m_created = other.m_created;
			if (!m_parent) m_parent = other.m_parent;
			m_predefined = other.m_predefined;
	
			if (m_created && m_predefined) call("@copy");
		};

		void steal(This& other){
			auto &map = other.m_list;
			m_list.clear();
			m_list = map;
			map.setRemover([&](const List::Id& id, const List::Type &obj){}); // this object does not own objects anymore => no deletion
			map.clear();
			map.defaultRemover();

			if (!m_created) m_created = other.m_created;
			m_predefined = other.m_predefined;

			if (m_created && m_predefined) call("@steal");
		};

		void swap(This& other){
			auto &map = other.m_list;
			auto saved = m_list;

			m_list.setRemover([&](const List::Id& id, const List::Type &obj){});
			m_list = map;
			m_list.defaultRemover();

			map.setRemover([&](const List::Id& id, const List::Type &obj){});
			map = saved;
			map.defaultRemover();

			saved.setRemover([&](const List::Id& id, const List::Type &obj){});

			auto d = m_predefined;
			m_predefined = other.m_predefined;
			other.m_predefined = d;

			if (m_created && m_predefined) call("@swap");
		};

		void clear(){ m_list.clear(); }

		Component& parent() { return *m_parent; };

		void create() { m_created = true; };
		void restart() { m_created = false; }
	
		void preset() {
			if (m_created || m_predefined) return;
			m_predefined = true;
			m_created = true;
			add<string>("@name");
			add<void()>("@create") = [](This*){};
			add<void()>("@delete") = [](This*){};
			add<void()>("@copy") = [](This*){};
			add<void()>("@steal") = [](This*){};
			add<void()>("@swap") = [](This*){};
			m_created = false;
		};

		Component() = default;
		~Component() { if (m_created && m_predefined) call("@delete"); };
	};
};

#endif