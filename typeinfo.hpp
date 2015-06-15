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
#ifndef __OPENEFW_TYPE_INFO_HPP__
#define __OPENEFW_TYPE_INFO_HPP__

#include <string>

namespace OpenEFW
{
	using ::std::type_info;
	using ::std::size_t;
	using ::std::string;
	using ::std::to_string;

	template<typename T = void> struct TypeInfo
	{
		static const type_info& type_info() { return typeid(T); }
		static inline size_t hash_code() { static size_t hash_code = type_info().hash_code(); return hash_code; }
		static inline const char* c_str() { static const char* name = type_info().name(); return name; }
		static inline string str() { static string name = clean(c_str()); return name; }

	protected:
		static inline string clean(const string& str) {
			string target = str;
			string::size_type index = 0;
			string::size_type invalid = string::npos;

			string s = "__cdecl"; index = target.find(s);
			if (index != invalid) target.replace(index, s.length(), "");

			index = target.find("::");

			if (index != invalid)
			{
				decltype(index) begin = target.substr(0, index).rfind(" ") + 1;
				decltype(index) end = target.rfind("(") - begin;
				target = target.substr(begin, end);
			}

			return target;
		};
	};

	template<> const char* TypeInfo<string>::c_str() { static const char* name = "std::string"; return name; }
};

#endif