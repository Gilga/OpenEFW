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

		virtual Value createValue() { return Value(); };
		virtual Type getValue(const Value& v) = 0;
		virtual void setValue(Value& v, const Type& obj) = 0;

	public:
		ExtendedMap() {}
		~ExtendedMap() { clear(); }

		size_t size() const { return m_map.size(); };

		void clear(){ if (m_map.size()) m_map.clear(); };

		bool has(Id id){ return m_map.find(id) != m_map.end(); }

		template<typename Class = T, typename ...Args> bool create(Id id, Args... args)
		{
			
			if (!is_convertible<Class*, Type>::value) OpenEFW_EXCEPTION(This, "create(!is_convertible)");
			if (!has(id)) { m_map.insert(Pair(id, Value(new Class(forward<Args>(args)...)))); return true; }
			return false;
		};

		virtual bool add(Id id, const Type& obj)
		{
			//if (!obj) OpenEFW_EXCEPTION(This, "add(NULL)");
			if (!has(id)) { m_map.insert(Pair(id, Value(obj))); return true; }
			return false; // failed, because it already exists
		};

		virtual bool replace(Id id, const Type& obj)
		{
			//if (!obj) OpenEFW_EXCEPTION(This, "replace(NULL)");
			auto it = m_map.find(id);
			if (it != m_map.end()) { setValue(it->second, obj); return true; }
			return false; // failed, because could not found
		};

		virtual auto get(Id id)->Type
		{
			auto it = m_map.find(id);
			if (it != m_map.end()) return getValue(it->second);

			//OpenEFW_EXCEPTION(This, "GET() -> NOT FOUND");
			return getValue(createValue());
		};

		virtual auto getByIndex(const size_t id = 0)->Type
		{
			unsigned int index = 0;
			Value& lib = createValue();

			bool found = loop([&](Id name, Type e) {
				bool found = (index == id);
				if (found) setValue(lib,e); else ++index;
				return found;
			});

			//if (!found) OpenEFW_EXCEPTION(This, "GET() -> NOT FOUND");
			return getValue(lib);
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
			for (auto & e : m_map) if (getValue(e.second) == obj)
			{
				m_map.erase(e.first);
				return true;
			};

			return false;
		};

		virtual bool loop(Function f)
		{
			for (auto & e : m_map) if (f(e.first, getValue(e.second))) return true; // break loop
			return false;
		}
	};

	template<typename I, typename T> class ExtendedMap<I, T> : public ExtendedMap<I, T, T, T>
	{
	public:
		using Id = I;
		using Type = T;
		using Value = T;

	protected:
		virtual Type getValue(const Value& v) { return v; };
		virtual void setValue(Value& v, const Type& obj) { v = obj; };
	};
};

#endif