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
#ifndef __OPENEFW_FUNCTIONLIST_HPP__
#define __OPENEFW_FUNCTIONLIST_HPP__

#include "SmartMap.hpp"

namespace OpenEFW
{
	class FunctionList {
	public:
		using BaseFunction = Delegate<>;
		template<typename T> using Function = Delegate<T>;
		using Map = SmartMap<string, BaseFunction>;

		inline void clear() { map.clear(); };

		// count of functions in list
		inline size_t count() const { return map.size(); };

		// get base function
		inline BaseFunction* get(const string &name) { return map.get(name); };

		// remove function
		inline bool remove(const string &name) { return map.del(name); };

		// get function
		template<typename T> inline Function<T>* function(const string &name) {
			string funcname = getFuncName<T>(name);
			BaseFunction *fb = get(funcname);
			return fb ? fb->get<T>() : nullptr;
		};

		// set function
		template<typename T, typename F> inline void set(const string &name, F func) {
			string funcname = getFuncName<T>(name);
			if (!get(funcname)) map.create<Function<T>>(funcname, func);
			else map.replace(funcname, new Function<T>(func));
		};

	protected:
		template<typename T> inline string getFuncName(const string& name) { return name + " " + TypeInfo<T>::str(); };

		Map map;
	};
};

#endif