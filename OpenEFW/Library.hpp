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
#ifndef __OPENEFW_LIBRARY_HPP__
#define __OPENEFW_LIBRARY_HPP__

#include "Component.hpp"
#include "VersionSet.hpp"

namespace OpenEFW
{
	template<typename T> class Library : public Component {
		SetUnknownClass

	public:
		using Type = T;
		using Super = Component;
		using This = Library<T>;
		using Versions = VersionSet<size_t>;

	protected:
		Versions versions;

	public:
		Library(){ m_typeinfo.set<T>(); };
		virtual ~Library(){};

		void setName(string const& name) { Super::add<string>(name); }; // set name of library
		void setVersion(Versions::List list) { versions.set(list); }; // set versions of library
		bool hasVersion(size_t version) { return versions.has(version); }; // check cetain version of library
		void useVersion(size_t version) { versions.use() = version; }; // check version of library

		bool checkVersion(bool exceptionOnFail = false)  // check current version of library
		{
			bool has = versions.is_available();
			if (!has && exceptionOnFail) THROW_EXCEPTION(Versions, "Version " + to_string(versions.use()) + " is not available for lib " + m_typeinfo.type_name());
			return has;
		}

		template<typename I, typename T> Delegate<T>& get() { return Super::get<Tags<This, I>, T>(); };
		template<typename I, typename T> Delegate<T>& add() { return Super::add<Tags<This, I>, T>(); };
		template<typename I, typename J = void, typename T> Delegate<T>& replace() { return Super::replace<Tags<This, I>, Tags<This, J>, T>(); };
		template<typename I, typename R = void, typename ...A> R call(A... args) { return Super::call<Tags<This, I>, R, A...>(forward<A>(args)...); };
	};
};

#endif