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

#include "UnknownClass.hpp"
#include "VersionSet.hpp"
#include "FunctionList.hpp"
#include "ObjectGenerator.hpp"

namespace OpenEFW
{
	template<typename ...A> class Library;

	/*! A template class for libraries */
	template<> class Library<> : public UnknownClass {
		OpenEFW_SetCurrentClass

	public:
		using version_t = size_t;

		using This = Library<>;
		using Versions = VersionSet<version_t>;

		bool hasVersion(version_t version){ return versions.has(version); }; // check cetain version of library
		void useVersion(version_t version) { versions.use() = version; }; // check version of library

		bool checkVersion(bool exceptionOnFail = false)  // check current version of library
		{
			bool has = versions.is_available();
			if (!has && exceptionOnFail) THROW_EXCEPTION(Versions, "Version " + to_string(versions.use()) + " is not available for lib " + m_typeinfo.type_name());
			return has;
		}

		// count of functions in list
		inline size_t count() const { return list.count(); };

		// get base function
		inline FunctionList::BaseFunction* get(const string &name) { return list.get(name); };

		// remove function
		inline bool remove(const string &name) { return list.remove(name); };

		// set function
		template<typename T, typename F> inline void set(const string &name, F func) { list.set<T, F>(name, func); };

		// get function
		template<typename T> inline FunctionList::Function<T>* function(const string &name) { return list.function<T>(name); };

		// call function
		template<typename R = void, typename ...A> inline R call(const string &name, A... args) {
			auto f = function<R(A...)>(name);
			if (!f) THROW_EXCEPTION(R(A...), "function " + name + " not found in lib " + m_typeinfo.type_name());
			return (*f)(forward<A>(args)...);
		};

		template<typename T> Library<T>* get()
		{
			if (m_typeinfo.hash_code() == TypeInfo<T>::hash_code()) return static_cast<Library<T>*>(this);
			return nullptr;
		};

		virtual ~Library(){ Destructor(); };

		virtual void initialize() {};
		virtual void terminate() {};

	protected:
		virtual void Destructor() { clear(); };

		inline void clear() { list.clear(); versions.clear(); };

		Versions versions;
		FunctionList list;
	};

	template<typename T> struct Library<T> : public Library<>{
		using Type = T;
		using Super = Library<>;
		using This = Library<T>;

		Library(){ m_typeinfo.set<T>(); };
		virtual ~Library(){};
	};
};

#endif