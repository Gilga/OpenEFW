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
#ifndef __OPENEFW_HASHKEY_HPP__
#define __OPENEFW_HASHKEY_HPP__

#include "type_string.hpp"
#include "type_functional.hpp"

namespace OpenEFW
{
	struct HashKey : public binary_function<HashKey, HashKey, bool>
	{
	private:
		using This = HashKey;
		size_t id;
		string content;

	public:

		template<typename T>
		static This create(string const& id) { return This(id + "_" + TypeInfo::Get<T>::to_str()); }

		This() = default;
		This(string const& c) : id(hash<string>()(c)), content(c) {};

		size_t to_hash() const { return id; };
		string to_str() const { return content; };

		void copy(const This &other) { id = other.id; content = other.content; };

		static bool compare(const This& left, const This& right) { return left == right; };

		This& operator=(const This &other) { copy(other); return *this; };
		bool operator==(const This &other) const { return (to_hash() == other.to_hash()); };
		bool operator<(const This &other) const { return (to_hash() < other.to_hash()); };
		bool operator()(const This& lhs, const This& rhs) const { return lhs == rhs; };
		size_t operator()(const This& k) const { return to_hash(); };
	};
};

#endif