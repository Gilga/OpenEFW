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
#include <iostream>

#include "Collection.hpp"
#include "Delegate.hpp"
#include "SmartMap.hpp"

namespace OpenEFW {
	using ::std::binary_function;
	using ::std::ostream;

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
		template<typename ...> struct ComponentFunction;
		template<typename T, typename ...A> struct ComponentFunction<T(A...)> { using Type = Delegate<T(This*, A...)>; };

	public:
		template<typename T> using Function = typename ComponentFunction<T>::Type;
		template<typename T> using Components = Collection<Key, T, Key, unordered_map<Key, T, Key>>;
		
		using List = SmartMap<size_t, Collection<>>;

	protected:
		List m_list;
		Key createKey(Identifer id) { return{ id }; };

	public:

		template<typename I = void> struct Static
		{
		protected:
			template<typename I, typename T> static inline Function<T>& function() { static Function<T> func; return func; };

			template<typename T> static inline string geType() {
				static auto str = TypeInfo::Get<I>::str() + "::" + TypeInfo::Get<T>::str();
				return str;
			};

		public:
			//template<typename I, typename T> static inline Function<T>& function() { static Function<T> func; return func; };

			template<typename T> static inline void set(Function<T> func) {
				auto& current = function<I, T>();
				if (current) OpenEFW_EXCEPTION(This, geType<T>() + " function already exists");
				current = func;
			};

			template<typename J = void, typename T> static inline enable_if_t<!is_same<I, J>::value> replace(Function<T> newfunc) {
				auto& func = function<I, T>();
				if (!func) OpenEFW_EXCEPTION(This, geType<T>() + " function does not exists");
				auto& oldfunc = function<J, T>();
				oldfunc = func;
				func = newfunc;
			};

			template<typename R = void, typename ...A> static inline R call(This* obj, A... args) {
				if (!obj) OpenEFW_EXCEPTION(This, geType<R(A...)>() + " object is null");
				auto f = function<I, R(A...)>();
				if (!f) OpenEFW_EXCEPTION(This, geType<R(A...)>() + " function does not exists");
				return (f)(obj, forward<A>(args)...);
			};
		};
	
		template<typename T> Components<T>& list(bool create = false){
			static auto hash_code = TypeInfo::Get<T>::hash_code();
			bool result = m_list.has(hash_code);
			if (!result && create) {
				if (!m_list.create<Components<T>>(hash_code))
					OpenEFW_EXCEPTION(This, TypeInfo::Get<T>::str() + " list could not be created");
				result = true;
			}
			if (!result) OpenEFW_EXCEPTION(This, TypeInfo::Get<T>::str() + " list does not exists");
			auto mlist = m_list.get(hash_code);
			auto nlist = mlist ? mlist->get<Components<T>>() : nullptr;
			if (!nlist){
				OpenEFW_EXCEPTION(This, TypeInfo::Get<T>::str() + " list weird error");
			}
			return *nlist;
		}

		template<typename T> T& replaceValue(Identifer id, Identifer old_id) {
			auto &key = createKey(id);
			auto &klist = list<T>(true);

			auto &value = klist.has(key) ? klist.get(key, true) : klist.add(key, true);
			if (old_id != id) addValue<T>(old_id) = value; // old

			return value;
		}

		template<typename T> Function<T>& replaceValue(Identifer id) { return replaceValue<Function<T>>(id, id); }
		template<typename T> This& replaceValue(Identifer id) { return replaceValue<This>(id, id); }

		template<typename T> T& addValue(Identifer id) { return list<T>(true).add(createKey(id)); }
		template<typename T> bool delValue(Identifer id) { return list<T>().del(createKey(id)); }
		template<typename T> T& getValue(Identifer id) { return list<T>().get(createKey(id)); }

		This& replaceComponent(Identifer id, Identifer old_id) { return replaceValue<This>(id, old_id); }
		This& replaceComponent(Identifer id) { return replaceValue<This>(id); }
		This& addComponent(Identifer id) { return addValue<This>(id); }
		bool delComponent(Identifer id) { return delValue<This>(id); }
		This& getComponent(Identifer id) { return getValue<This>(id); }

		template<typename T> Function<T>& replaceFunction(Identifer id, Identifer old_id) { return replaceValue<Function<T>>(id, old_id); }
		template<typename T> Function<T>& replaceFunction(Identifer id) { return replaceValue<Function<T>>(id); }
		template<typename T> Function<T>& addFunction(Identifer id) { return addValue<Function<T>>(id); }
		template<typename T> bool delFunction(Identifer id) { return delValue<Function<T>>(id); }
		template<typename T> Function<T>& getFunction(Identifer id) { return getValue<Function<T>>(id); }

		template<typename T = void, typename ...A> T call(Identifer id, A... args){ return getFunction<T(A...)>(id)(this, forward<A>(args)...); };
		template<typename T = void, typename ...A> T invoke(Function<T(A...)> func, A... args){ return func(this, forward<A>(args)...); };

		template<typename T> enable_if_t<!is_same<This, typename decay<T>::type>::value, T>&
			add(Identifer id){ return addValue<T>(id); }

		template<typename T> enable_if_t<!is_same<This, typename decay<T>::type>::value, T>&
			del(Identifer id){ return delValue<T>(id); }

		template<typename T> enable_if_t<!is_same<This, typename decay<T>::type>::value, T>&
			get(Identifer id){ return getValue<T>(id); }

		template<typename T> enable_if_t<is_same<This, typename decay<T>::type>::value, This>&
			add(Identifer id){ return addComponent(id); }

		template<typename T> enable_if_t<is_same<This, typename decay<T>::type>::value, bool>
			del(Identifer id){ return delComponent(id); }

		template<typename T> enable_if_t<is_same<This, typename decay<T>::type>::value, This>&
			get(Identifer id){ return getComponent(id); }

		template<typename T> enable_if_t<is_same<Function<T>, typename decay<T>::type>::value, Function<T>>&
			add(Identifer id){ return addFunction<T>(id); }

		template<typename T> enable_if_t<is_same<Function<T>, typename decay<T>::type>::value, Function<T>>&
			del(Identifer id){ return delFunction<T>(id); }

		template<typename T> enable_if_t<is_same<Function<T>, typename decay<T>::type>::value, Function<T>>&
			get(Identifer id){ return getFunction<T>(id); }

		template<typename T> struct In {
			Identifer id;
			T value;
			In(const Identifer &id, const T &value) : id(id), value(value) {};
		};

		template<typename T> This& operator=(const In<Function<T>> in){ replaceFunction(in.id) = in.value; return *this; };
		template<typename T> This& operator=(const In<T> in){ replaceValue(in.id) = in.value; return *this; };

		This& operator=(const In<This>& in){ replaceComponent(in.id) = in.value; return *this; };
		This& operator=(const This& other){ copy(const_cast<This&>(other)); return *this; };

		friend ostream & operator<<(ostream &os, const This& o) { return os << "Component"; };

		void copy(This& other){
			auto &map = other.m_list;
			m_list.clear();
			map.loop([&](List::Id id, List::Type type){ m_list.add(id, type->copy()); return false; });
			//for (auto &e : map.source()) m_list.add(e.first, e.second.get()->copy());
		};

		void steal(This& other){
			auto &map = other.m_list;
			m_list.clear();
			m_list = map;
			map.setRemover([&](const List::Id& id, const List::Type &obj){}); // this object does not own objects anymore => no deletion
			map.clear();
			map.defaultRemover();
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
		};

		void clear(){ m_list.clear(); }

		Component() {};
		Component(const This& other) : m_list(other.m_list) {};
	};
};

#endif