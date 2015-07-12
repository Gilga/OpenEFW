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
#ifndef __OPENEFW_EXTENDED_MAP_HPP__
#define __OPENEFW_EXTENDED_MAP_HPP__

#include <unordered_map>

#include "delegate.hpp"
#include "macros/exception.hpp"

namespace OpenEFW
{
	template<typename ...> class ExtendedMap;

	template<typename I, typename T, typename VType, typename VValue> class ExtendedMap<I, T, VType, VValue>
	{
	public:
		using This = ExtendedMap<I, T, VType, VValue>;

		using Id = I;
		using Type = VType;
		using Value = VValue;

		using Function = Delegate<bool(Id, Type)>;
		using Map = unordered_map<Id, Value>;
		using Pair = pair<Id, Value>;

	protected:
		Map m_map;

		virtual Value createValue() = 0;
		virtual Value createValue(const Id& id) = 0;
		virtual Value createValue(const Id& id, const Type& obj) = 0;
		virtual Type getContent(const Value& v) = 0;
		virtual void replaceValue(const Id& id, Value& v, const Type& obj) = 0;

		Id getID(const Type& obj){
			for (auto &e : m_map) if(getContent(e.second) == obj) return e.first;
			OpenEFW_EXCEPTION(This, "getID() = not found!");
		}

	public:
		ExtendedMap() {}
		~ExtendedMap() { clear(); }

		This& operator=(const This &other){ m_map = other.m_map; return *this; };

		Map& source() { return m_map; };

		size_t size() const { return m_map.size(); };

		virtual void clear(){ if (size()) m_map.clear(); };

		bool has(Id id){ return m_map.find(id) != m_map.end(); }

		template<typename Class = T, typename ...Args> bool create(Id id, Args... args)
		{
			
			if (!is_convertible<Class*, Type>::value) THROW_EXCEPTION(This, "create(!is_convertible)");
			if (!has(id)) { m_map.insert(Pair(id, createValue(id, new Class(forward<Args>(args)...)))); return true; }
			return false;
		};

		virtual bool add(Id id, const Type& obj)
		{
			if (!has(id)) { m_map.insert(Pair(id, createValue(id, obj))); return true; }
			return false; // failed, because it already exists
		};

		virtual bool replace(Id id, const Type& obj)
		{
			//if (!obj) OpenEFW_EXCEPTION(This, "replace(NULL)");
			auto it = m_map.find(id);
			if (it != m_map.end()) { replaceValue(id, it->second, obj); return true; }
			return false; // failed, because could not found
		};

		virtual auto get(Id id)->Type
		{
			auto it = m_map.find(id);
			if (it != m_map.end()) return getContent(it->second);

			//OpenEFW_EXCEPTION(This, "GET() -> NOT FOUND");
			return getContent(createValue(id));
		};

		virtual auto getByIndex(const size_t id = 0)->Type
		{
			unsigned int index = 0;
			Value& lib = createValue();

			bool found = loop([&](Id name, Type e) {
				bool found = (index == id);
				if (found) replaceValue(name, lib, e); else ++index;
				return found;
			});

			//if (!found) OpenEFW_EXCEPTION(This, "GET() -> NOT FOUND");
			return getContent(lib);
		};

		virtual bool delByIndex(const size_t id = 0)
		{
			unsigned int index = 0;
			return loop([&](Id name, Type e) {
				bool found = (index == id);
				if (found) m_map.erase(name); else ++index;
				return found;
			});
		};

		virtual bool del(Id id) {
			auto it = m_map.find(id);
			if (it != m_map.end()) { m_map.erase(it); return true; }
			return false;
		};

		virtual bool del(const Type& obj)
		{
			//if (!obj) OpenEFW_EXCEPTION(This, "remove(NULL)");
			for (auto & e : m_map) if (getContent(e.second) == obj)
			{
				m_map.erase(e.first);
				return true;
			};

			return false;
		};

		virtual bool loop(Function f)
		{
			for (auto & e : m_map) if (f(e.first, getContent(e.second))) return true; // break loop
			return false;
		}
	};
};

#endif