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
#ifndef __OPENEFW_OBJECT_GENERATOR_HPP__
#define __OPENEFW_OBJECT_GENERATOR_HPP__

#include "Delegate.hpp"

namespace OpenEFW
{
	template<typename C> class ObjectGenerator
	{
	public:
		using shared_ptr = shared_ptr<C>;

		struct null_delete { void operator()(void const *) const {} };

		// replace creation function of object
		template<typename ...A> static Delegate<shared_ptr(A...)>& delegate(){
			static Delegate<shared_ptr(A...)> d = default_new<C, A...>(); 
			return d;
		};

		static void reset() { pointer().reset(); }

		template<typename ...A> static shared_ptr singleton(A... args) _NOEXCEPT{
			if (!pointer()) pointer() = delegate()(forward<A>(args)...);
			return shared_ptr(pointer().get(), null_delete{});
		};

		template<typename ...A> static shared_ptr instance(A... args) _NOEXCEPT{
			shared_ptr ptr;
			pointer() ? ptr.reset(pointer().get(), null_delete{}) : ptr = delegate()(forward<A>(args)...);
			return ptr;
		};

	protected:
		static shared_ptr& pointer() { static shared_ptr c; return c; }

		template<typename C, typename ...A> static enable_if_t<is_constructible<C,A...>::value, Delegate<shared_ptr(A...)>> default_new() {
			return [](A... args){
				return shared_ptr(new C(forward<A>(args)...));
			};
		};

		template<typename C, typename ...A> static enable_if_t<!is_constructible<C, A...>::value, Delegate<shared_ptr(A...)>> default_new() {
			return nullptr;
		};
	};
};

#endif