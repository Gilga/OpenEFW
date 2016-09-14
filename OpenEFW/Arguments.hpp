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
#ifndef __OPENEFW_ARGUMENTS_HPP__
#define __OPENEFW_ARGUMENTS_HPP__

#ifndef ARGS2
#define ARGS2(x,y) ::OpenEFW::Arguments<decltype(x), decltype(y)>{x,y};
#endif

#ifndef SARGS2
#define SARGS2(x,y) ::OpenEFW::Arguments<::OpenEFW::string, decltype(y)>{x,y};
#endif

namespace OpenEFW
{
	template<typename ...> struct Arguments;
	
	template<typename T> struct Arguments<T> {
		const T &a;
	};

	template<typename T1, typename T2> struct Arguments<T1, T2> {
		const T1 &a1;
		const T2 &a2;
	};

	template<typename T1, typename T2, typename T3> struct Arguments<T1, T2, T3> {
		const T1 &a1;
		const T2 &a2;
		const T3 &a3;
	};

	template<typename T1, typename T2, typename T3, typename T4> struct Arguments<T1, T2, T3, T4> {
		const T1 &a1;
		const T2 &a2;
		const T3 &a3;
		const T4 &a4;
	};

	template<typename T1, typename T2, typename T3, typename T4, typename T5> struct Arguments<T1, T2, T3, T4, T5> {
		const T1 &a1;
		const T2 &a2;
		const T3 &a3;
		const T4 &a4;
		const T5 &a5;
	};
};

#endif