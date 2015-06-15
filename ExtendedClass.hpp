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
#ifndef __OPENEFW_EXTENDEDCLASS_HPP__
#define __OPENEFW_EXTENDEDCLASS_HPP__

#include "BaseClass.hpp"
#include "macros/exception.hpp"

namespace OpenEFW
{
	template<typename T> class ExtendedClass : public BaseClass {
	public:
		using This = ExtendedClass<T>;
		using Values = T;

		template<typename ...> struct ExtendedClassFunction;
		template<typename T, typename ...A> struct ExtendedClassFunction<T(A...)> { using Type = Delegate<T(Values&, A...)>; };

		template<typename T> using Function = typename ExtendedClassFunction<T>::Type;

		template<typename I, typename T> inline Function<T>& function() {
			static Function<T> func;
			return func;
		};

		template<typename I, typename R = void, typename ...A> inline R call(A... args) {
			auto f = function<I,R(A...)>();
			if (!f) OpenEFW_EXCEPTION(I(R,A...), "function is empty in ExtendedClass " + str());
			return (f)(values, forward<A>(args)...);
		};

		virtual string str() {
			if(type[0] == '?') setType<T>();
			return "[" + type_name() + "]";
		};

	private:
		Values values;
	};
};

#endif