/*
 * Copyright (c) 2017, Mario Link
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
#ifndef __OPENEFW_CONTAINER_HPP__
#define __OPENEFW_CONTAINER_HPP__

#include "UnknownClass.hpp"

namespace OpenEFW
{
	template<typename ...A> class Container;

	template<> class Container<> : public UnknownClass
	{
		Container(Container const&) = delete;
		Container& operator=(Container const&) = delete;

		string m_description = "";

	protected:
		Container() {}

	public:
		using This = Container<>;

		template<typename T> Container<T>* cast()
		{
			using C = Container<T>;
			if (m_typeinfo.hash_code() != TypeInfo::Get<C>::hash_code()) return nullptr;
			return static_cast<C*>(this);
		}

		string& description() { return m_description; };
	};

	template<typename T> class Container<T> : public Container<>
	{
		SetUnknownClass

	public:
		using Type = T;
		using This = Container<T>;
		using Super = Container<>;

		template<typename ...Args>
		Container(Args... args) : value(Type(forward<Args>(args)...)) { self_update(); }
		
		Type value;

		This& operator=(This &other) = delete;
		This& operator=(const Type &other) { value = other; return *this; }
		Type* operator->() { return &value; }
		Type& operator*() { return value; }
	};
};

#endif