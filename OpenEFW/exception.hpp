/*
 * Copyright (c) 2016, Mario Link
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
#ifndef __OPENEFW_EXCEPTION_HPP__
#define __OPENEFW_EXCEPTION_HPP__

#include "macros/exception.hpp"
#include "type_exception.hpp"
#include "typeinfo.hpp"

namespace OpenEFW
{
	namespace ExceptionSettings {
		static bool& showClass() { static bool show = true; return show; };
		static bool& showLine() { static bool show = false; return show; };
	};

	template<typename T> class Exception : public exception
	{
	public:
		Exception(const string& msg, const string& file = "?", const int line = 0) : msg(msg) {
			this->line = " @ " + file + " (" + to_string(line)+")";
		};

		~Exception() throw () {};

		const char* what() const throw() {
			static string tmp = "";
			bool show = ExceptionSettings::showClass();

			tmp = "[ Exception";
			if (msg!="") tmp += " : " + msg;
			tmp += " ]";
			if (ExceptionSettings::showClass()) tmp += " # " + type;
			if (ExceptionSettings::showLine()) tmp += line;
			tmp += "\n";

			return tmp.c_str();
		};

	protected:

		string type = TypeInfo::Get<T>::to_str();
		string msg;
		string line;
	};
};

#endif