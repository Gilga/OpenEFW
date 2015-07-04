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
	using ::std::binary_function;
	using ::std::ostream;

	template<typename T = void, typename Identifer = string> struct Component {
		using Type = T;
		using This = Component<T>;

		struct Key : public binary_function<Key, Key, bool>
		{
			size_t first = 0; //This*
			Identifer second;

			Key() = default;
			Key(size_t parent, Identifer id) : first(parent), second(id) {};

			static bool compare(const Key& x, const Key& y) { return x == y; };
			static string toStr(Key& k) { return "[" + to_string(k.first) + "][" + k.second + "]"; }; // TypeInfo::Get<decltype(k.first)>::str()

			Key& operator=(const Key &other) const { return *this; };
			bool operator==(const Key &other) const	{ return (first == other.first && second == other.second); };
			bool operator<(const Key &other) const	{ return first < other.first || (first == other.first && second.length() < other.second.length()); };
			size_t operator()(const Key& k) const { return hash<decltype(k.first)>()(k.first) ^ (hash<decltype(k.second)>()(k.second) << 1); };
			bool operator()(const Key& lhs, const Key& rhs) const { return lhs == rhs; }; // lhs < rhs  -> not right yet
		};

	protected:
		template<typename ...> struct ComponentFunction;
		template<typename T, typename ...A> struct ComponentFunction<T(A...)> { using Type = Delegate<T(This*, A...)>; };

	public:
		template<typename T> using Function = typename ComponentFunction<T>::Type;
		template<typename T> using Components = Collection<Key, T, Key, unordered_map<Key, T, Key>>;
		
		using List = SmartMap<size_t,Collection<>>;

	protected:
		List m_list;

		Key createKey(Identifer id) { return{ 0, id }; };

		template<typename I, typename T> static inline Function<T>& function() {
			static Function<T> func;
			return func;
		};

	public:

		template<typename I = void> struct Static
		{
		protected:
			template<typename T> static inline string geType() {
				static auto str = TypeInfo::Get<I>::str() + "::" + TypeInfo::Get<T>::str();
				return str;
			};

		public:
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

		template<typename T> T& ReplaceValue(Identifer id, Identifer old_id) {
			auto &key = createKey(id);
			auto &klist = list<T>();

			auto &value = klist.has(key) ? klist.get(key, true) : klist.add(key, true);
			if (old_id != id) AddValue<T>(old_id) = value; // old

			return value;
		}

		template<typename T> Function<T>& ReplaceValue(Identifer id) { return ReplaceValue<Function<T>>(id, id); }

		template<typename T> T& AddValue(Identifer id) { return list<T>(true).add(createKey(id)); }
		template<typename T> T& DelValue(Identifer id) { return list<T>().del(createKey(id)); }
		template<typename T> T& GetValue(Identifer id) { return list<T>().get(createKey(id)); }

		template<typename T> Component<T>& ReplaceComponent(Identifer id, Identifer old_id) { return ReplaceValue<Component<T>>(id, old_id); }
		template<typename T> Component<T>& ReplaceComponent(Identifer id) { return ReplaceValue<Component<T>>(id); }
		template<typename T> Component<T>& AddComponent(Identifer id) { return AddValue<Component<T>>(id); }
		template<typename T> Component<T>& DelComponent(Identifer id) { return DelValue<Component<T>>(id); }
		template<typename T> Component<T>& GetComponent(Identifer id) { return GetValue<Component<T>>(id); }

		template<typename T> Function<T>& ReplaceFunction(Identifer id, Identifer old_id) { return ReplaceValue<Function<T>>(id, old_id); }
		template<typename T> Function<T>& ReplaceFunction(Identifer id) { return ReplaceValue<Function<T>>(id); }
		template<typename T> Function<T>& AddFunction(Identifer id) { return AddValue<Function<T>>(id); }
		template<typename T> Function<T>& DelFunction(Identifer id) { return DelValue<Function<T>>(id); }
		template<typename T> Function<T>& GetFunction(Identifer id) { return GetValue<Function<T>>(id); }

		template<typename T = void, typename ...A> T Call(Identifer id, A... args){ return GetFunction<T(A...)>(id)(this, forward<A>(args)...); };
		template<typename T = void, typename ...A> T Invoke(Function<T(A...)> func, A... args){ return func(this, forward<A>(args)...); };

		template<typename T> enable_if_t<!is_same<Component, typename decay<T>::type>::value, T>&
			Add(Identifer id){ return AddValue<T>(id); }

		template<typename T> enable_if_t<!is_same<Component, typename decay<T>::type>::value, T>&
			Del(Identifer id){ return DelValue<T>(id); }

		template<typename T> enable_if_t<!is_same<Component, typename decay<T>::type>::value, T>&
			Get(Identifer id){ return GetValue<T>(id); }

		template<typename T> enable_if_t<is_same<Component, typename decay<T>::type>::value, Component<T>>&
			Add(Identifer id){ return AddComponent<T>(id); }

		template<typename T> enable_if_t<is_same<Component, typename decay<T>::type>::value, Component<T>>&
			Del(Identifer id){ return DelComponent<T>(id); }

		template<typename T> enable_if_t<is_same<Component, typename decay<T>::type>::value, Component<T>>&
			Get(Identifer id){ return GetComponent<T>(id); }

		template<typename T> enable_if_t<is_same<Function<T>, typename decay<T>::type>::value, Function<T>>&
			Add(Identifer id){ return AddFunction<T>(id); }

		template<typename T> enable_if_t<is_same<Function<T>, typename decay<T>::type>::value, Function<T>>&
			Del(Identifer id){ return DelFunction<T>(id); }

		template<typename T> enable_if_t<is_same<Function<T>, typename decay<T>::type>::value, Function<T>>&
			Get(Identifer id){ return GetFunction<T>(id); }

		//template<typename T> This& operator=(T other){ value = other; return *this; };
		//friend ostream & operator<<(ostream &os, const This& p) { return os << p.value; };
	};
};

#endif