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
#ifndef __OPENEFW_COLLECTION_HPP__
#define __OPENEFW_COLLECTION_HPP__

#include <unordered_map>
#include <map>
#include <set>

#include "exception.hpp"
#include "macros/exception.hpp"

#include "UnknownClass.hpp"

namespace OpenEFW {

	using ::std::map;
	using ::std::set;

	template<typename ...A> class Collection;

	template<> class Collection<> : public UnknownClass {
	public:
		using This = Collection<>;

		template<typename T> Collection<T>* cast(){
			using Derived = Collection<T>;
			if (m_typeinfo.hash_code() != TypeInfo::Get<T>::hash_code()) return nullptr;
			return static_cast<Derived*>(this);
		}

		virtual void copy(Collection<>& other) = 0;
		virtual This* copy() = 0;
	};

	template<typename K, typename T, typename H, typename M, typename S> class Collection<K, T, H, M, S> : public Collection<>{
		OpenEFW_SetCurrentClass

	public:
		using Key = K;
		using Hasher = H;
		using Type = T;
		using Map = M;
		using Set = S;

		using This = Collection<Key, Type, Hasher, Map, Set>;
		using Super = Collection<>;
		using Index = size_t;

		Collection() { m_typeinfo.set<remove_pointer<decltype(this)>::type>(); }

		size_t size() { return map.size(); };

		void clear() { return map.clear(); };

		bool has(Key id){ return size() && map.find(id) != map.end(); }

		Type& first(){
			if (!size()) THROW_EXCEPTION(This, ": has no entry '" + m_typeinfo.type_name() + "'");
			return map.begin()->second;
		}

		Type& getByIndex(Index id){
			if (id >= size()) THROW_EXCEPTION(This, "invalid index " + to_string(id) + " for entries '" + m_typeinfo.type_name() + "'");
			Index index = 0; for (auto& e : map) { if (index == id) { return e.second; }; ++index; }
			OpenEFW_EXCEPTION(This, "weird error for entries '" + m_typeinfo.type_name() + "'");
		}

		Type& get(Key id, bool ignore = false){
			if (!ignore && !has(id)) THROW_EXCEPTION(This, "could not find entry " + Hasher::toStr(id) + " in " + m_typeinfo.type_name());
			return map[id];
		}

		Type& add(Key id, bool ignore = false){ if (ignore || !has(id)) { list.insert(id), map[id] = map[id]; }; return get(id, ignore); }

		bool del(Key id, bool ignore = false){ if (ignore || has(id)) { map.erase(id), list.erase(id); }; return !has(id); }

		bool copy(Key id, bool ignore = false){ if (ignore || has(id)) { map.erase(id), list.erase(id); }; return !has(id); }

		Set list;
		Map map;

		virtual void copy(Collection<>& other) {
			auto ref = other.get<Collection<K, T, H, M, S>>();
			if (ref) map = ref->map;
		}

		virtual Super* copy() { This *c = new This(); c->list = list; c->map = map; return c; };
	};

	template<typename Key, typename T, typename Hasher, typename Map> class Collection<Key, T, Hasher, Map>
		: public Collection<Key, T, Hasher, Map, set<Key, Hasher>> {
			OpenEFW_SetCurrentClass
	public:
		using This = Collection<Key, T, Hasher, Map>;
		using Base = Collection<>;
		Collection() { m_typeinfo.set<remove_pointer<decltype(this)>::type>(); }
		virtual Base* copy() { This *c = new This(); c->list = list; c->map = map; return c; };
	};

	template<typename Key, typename T, typename Hasher> class Collection<Key, T, Hasher>
		: public Collection<Key, T, Hasher, map<Key, T, Hasher>, set<Key, Hasher>>{
			OpenEFW_SetCurrentClass
	public:
		using This = Collection<Key, T, Hasher>;
		using Base = Collection<>;
		Collection() { m_typeinfo.set<remove_pointer<decltype(this)>::type>(); }
		virtual Base* copy() { This *c = new This(); c->list = list; c->map = map; return c; };
	};

	template<typename Key, typename T> class Collection<Key, T> : public Collection<Key, T, Key>{
		OpenEFW_SetCurrentClass

	public:
		using This = Collection<Key, T>;
		using Base = Collection<>;
		Collection() { m_typeinfo.set<remove_pointer<decltype(this)>::type>(); }
		virtual Base* copy() { This *c = new This(); c->list = list; c->map = map; return c; };
	};
};

#endif