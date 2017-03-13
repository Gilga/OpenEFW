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
#ifndef __OPENEFW_LIBRARY_HPP__
#define __OPENEFW_LIBRARY_HPP__

#include "Component.hpp"
#include "VersionSet.hpp"

namespace OpenEFW
{
	using Versions = VersionSet<size_t>;

	template<typename LibT>
	struct Library
	{
		using LibraryType = LibT;
		using Type = Component;
		using This = Type*;

		static void init(This);

		static auto create(const string &pname)
		{
			shared_ptr<Component> ptr(new Component());

			ptr->setID(TypeInfo::Get<Library<LibT>>::to_str());

			auto& versions = ptr->add<Versions>("versions");
			auto& name = ptr->add<string>("name");
			name = pname;

			ptr->add("version", [&](This, Versions::List list) { versions.set(list); });
			ptr->add("version", [&](This, size_t version) { return versions.has(version); });
			ptr->add("version", [&](This, size_t version) { versions.use() = version; });
			ptr->add("version", [&](This c, bool exceptionOnFail = false)  // check current version of library
			{
				bool has = versions.is_available();
				if (!has && exceptionOnFail)
					THROW_EXCEPTION(Versions, "Version " + to_string(versions.use()) + " is not available for library " + c->to_str()) + "(" + name + ")";
				return has;
			});

			init(ptr.get());

			return ptr;
		}
	};
};

#endif