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
#ifndef __OPENEFW_UNKNOWNCLASS_HPP__
#define __OPENEFW_UNKNOWNCLASS_HPP__

#ifndef SetUnknownClass
#define SetUnknownClass private: virtual void set() { _unknownClass<RemovePointer(this)>() = this; };
#endif

#include "exception.hpp"
#include "type_thread.hpp"

namespace OpenEFW
{
	/*
	 *	This class retuns a derivated class without casting
	 */
	class UnknownClass {

		UnknownClass(UnknownClass const&) = delete;
		UnknownClass& operator=(UnknownClass const&) = delete;

	private:

		// for thread safety
		static mutex& _mutex(){ static mutex m; return m; };

	protected:
		UnknownClass() {}

		TypeInfo m_typeinfo;

		// saves and returns the current object
		template<typename T> static T*& _unknownClass(){
			static T* obj = nullptr;
			return obj;
		};

		// sets the current object
		virtual void set() = 0;
		virtual void Destructor() {};	// Destructor
		//virtual void Constructor() {};	// Constructor
		
		template<class T, typename = enable_if_t<is_base_of<UnknownClass, T>::value> >
		void updateTypeInfo(T* t)
		{
			if((UnknownClass*)t != this)
				THROW_EXCEPTION(T, ": is not derivative of this object (UnknownClass)");
			
			m_typeinfo.set<RemovePointer(t)>();
		}

	public:
		virtual TypeInfo getTypeInfo() { return m_typeinfo; };
		virtual string to_str() { return "(Unknown Class)"; };
	
		// returns current object, thread secure
		template<typename T> T* reconvert(){
			lock_guard_mutex guard(_mutex());
			set();
			T*& uobj = _unknownClass<T>();
			T* obj = uobj;
			uobj = nullptr;
			return obj;
		};

		virtual ~UnknownClass() { Destructor(); }; // Destructor
	};
};

#endif