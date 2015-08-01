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
#ifndef __OPENEFW_DATAVALUEPTR_HPP__
#define __OPENEFW_DATAVALUEPTR_HPP__

//#include "type_memory.hpp"

#include "exception.hpp"
#include "typeinfo.hpp"

namespace OpenEFW
{
	template<typename ...> class ValuePtr;

	template<> class ValuePtr<> {
	protected:
		using This = ValuePtr<>;

		TypeInfo m_typeinfo;

		size_t m_size = 0;
		shared_ptr<void> m_data;

		struct null_delete { void operator()(void const *) const {} };

	public:
		TypeInfo getTypeInfo() { return m_typeinfo; };

		template<typename T> T* get()
		{
			if (hasType<T>()) return static_cast<T*>(m_data.get());
			return nullptr;
		}

		template<> void* get() { return m_data.get(); };

		template<typename T> bool hasType() const { return m_code == TypeInfo::Get<T>::hash_code(); };

		explicit operator bool() const _NOEXCEPT{ return m_data ? true : false; };
		bool operator==(nullptr_t const) const _NOEXCEPT{ return m_data == nullptr; };
		bool operator!=(nullptr_t const) const _NOEXCEPT{ return m_data != nullptr; };
	};

	template<typename T> class ValuePtr<T> : public ValuePtr<>{
	protected:
		using This = ValuePtr<T>;
		using Super = ValuePtr<>;

	public:

		ValuePtr(){ m_typeinfo.set<T>(); };
		ValuePtr(ValuePtr const&) = default;
		//ValuePtr(ValuePtr& v) { m_typeinfo.set<T>(); m_data = v.m_data; m_size = v.m_size; };

		//ValuePtr&operator=(ValuePtr& v) const _NOEXCEPT{
		//	m_data = v.m_data;
		//	v.m_data.reset();
		//	return *this;
		//};

		ValuePtr& operator=(ValuePtr const&) = default;

		template<typename P> enable_if_t<is_convertible<P*, T*>::value, ValuePtr&> operator=(P& v) _NOEXCEPT{
			m_data.reset(&v, null_delete{});
			m_size = sizeof(P);
			return *this;
		};

		template<typename P> enable_if_t<is_convertible<P*, T*>::value, ValuePtr&> operator=(P* v) _NOEXCEPT{
			m_data.reset(v);
			m_size = sizeof(P);
			return *this;
		};
	};

	using _value_ptr = ValuePtr<>;
};

#endif