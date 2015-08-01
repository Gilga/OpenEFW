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

#include "stringReplace.hpp"

namespace OpenEFW
{
	class TypeInfo {
	protected:
		size_t m_hash_code = 0;
		string m_type_name = "?";

	public:
		template<typename T> inline void set() {
			m_type_name = Get<T>::str();
			m_hash_code = Get<T>::hash_code();
		};

		template<typename T> bool hasType() const { return m_hash_code == TypeInfo::Get<T>::hash_code(); };

		virtual size_t hash_code() { return m_hash_code; };
		virtual string type_name() { return m_type_name; };

		template<typename T> struct Get
		{
			static const type_info& type_info() { return typeid(T); }
			static inline size_t hash_code() { static size_t hash_code = type_info().hash_code(); return hash_code; }
			static inline const char* c_str() { static const char* name = type_info().name(); return name; }
			static inline string str() { static string name = clean(c_str()); return name; }

			static inline string clean(string target) {
		
				String::replaceAll(target, typeid(string).name(), "std::string");
				String::replaceAll(target, "__cdecl", "");
				String::replaceAll(target, "class ", "");
				String::replaceAll(target, "struct ", "");
				String::replaceAll(target, " >", ">");
				String::replaceAll(target, "< ", "<");

				return target;
			};
		};
	};

	template<> const char* TypeInfo::Get<string>::c_str() { static const char* name = "std::string"; return name; }
};

#endif