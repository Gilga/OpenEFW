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
#ifndef __OPENEFW_SMART_MAP_HPP__
#define __OPENEFW_SMART_MAP_HPP__

#include "ExtendedMap.hpp"

namespace OpenEFW
{
	template<typename I, typename T> class SmartMap : public ExtendedMap<I, T, T*, shared_ptr<T>>{
		using This = SmartMap<I,T>;
		using Super = ExtendedMap<I, T, T*, shared_ptr<T>>;

	public:
		using typename Super::Id;
		using typename Super::Type;
		using typename Super::Value;

		virtual bool add(Id id, const Type& obj)
		{
			if (!obj) OpenEFW_EXCEPTION(This, "add(NULL)");
			return __super::add(id, obj);
		};

		virtual bool replace(Id id, const Type& obj)
		{
			if (!obj) OpenEFW_EXCEPTION(This, "replace(NULL)");
			return __super::replace(id, obj);
		};

		virtual bool del(Id id) { return __super::del(id); };

		virtual bool del(const Type& obj)
		{
			if (!obj) OpenEFW_EXCEPTION(This, "remove(NULL)");
			return __super::del(obj);
		};

	protected:
		virtual Type getValue(const Value& v) { return v.get(); };
		virtual void setValue(Value& v, const Type& obj) { v.reset(obj); };
	};
};

#endif