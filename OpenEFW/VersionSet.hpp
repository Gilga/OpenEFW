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
#ifndef __OPENEFW_VERSION_SET_HPP__
#define __OPENEFW_VERSION_SET_HPP__

#include <set>

#include "exception.hpp"
#include "macros/exception.hpp"

namespace OpenEFW
{
	template<typename T> class VersionSet {
	public:
		using Type = T;
		using List = std::set<Type>;

		struct Result {
			bool result = false;
			Type value = 0;
			Result(bool result = false, Type value = 0) : result(result), value(value) {};
		};

		// Version in use [version = 0 : independent; version != 0 : dependent]
		Type& use() { return current; };

		// set versions
		void set(List versions) { list = versions; };

		// add versions
		void add(List versions) {};

		// remove versions
		void remove(List versions) {};

		// add version
		void add(Type version) { if (has(version)) return; list.insert(version); };

		// delete version
		void del(Type version) { if (!has(version)) return; list.erase(_tmp()); };

		// clear all
		void clear() { if (active()) list.clear(); };

		// get
		Result get(size_t id, bool exceptionOnFail = false) {
			if (!active() || list.size() >= id) {
				if (exceptionOnFail) THROW_EXCEPTION(Type, "There is no such Version");
				return {};
			};
			id_t index = 0;
			for (auto e : list) { if (id == index) return { true, e }; ++index; };
			return {}; // not reachable
		};

		// get newest
		Result newest() { return active() ? Result(true, *list.end()) : Result(); };

		// get oldest
		Result oldest() { return active() ? Result(true, *list.begin()) : Result(); };

		// check version (Version == lib.Version)
		bool is_newest(Type version, bool exceptionOnFail = false)
		{
			auto r = newest();
			bool tmp = (version > 0 && r.result &&version == r.value);
			if (!tmp && exceptionOnFail) THROW_EXCEPTION(Type, "Version is not newest");
			return tmp;
		};

		// check version (Version == lib.Version)
		bool is_oldest(Type version, bool exceptionOnFail = false)
		{
			auto r = oldest();
			bool tmp = (version > 0 && r.result && version == r.value);
			if (!tmp && exceptionOnFail) THROW_EXCEPTION(Type, "Version is not oldest");
			return tmp;
		};

		// check version
		bool has(Type version, bool exceptionOnFail = false) {
			if (active()){
				_tmp() = list.find(version);
				if (_tmp() != list.end()) return true;
			}
			//for (auto e : list) if (e == version) return true;
			if (exceptionOnFail) THROW_EXCEPTION(Type, "Version is not available");
			return false;
		};

		// check if current is newest
		bool is_newest(bool exceptionOnFail = false) { return is_newest(current, exceptionOnFail); };

		// check if current is oldest
		bool is_oldest(bool exceptionOnFail = false) { return is_oldest(current, exceptionOnFail); };

		// check if current is available
		bool is_available(bool exceptionOnFail = false) { return has(current, exceptionOnFail); };

		// check version
		bool active() { return !list.empty(); };

	protected:
		T current = 0;
		List list;

	private:
		typename List::iterator& _tmp() { static List::iterator tmp = list.begin(); return tmp; };
	};
};

#endif