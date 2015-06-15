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
#ifndef __OPENEFW_HASH_MAP_HPP__
#define __OPENEFW_HASH_MAP_HPP__

#ifdef _USE_BOOST
#include <boost/multi_index/hashed_index_fwd.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>

#include <iostream>

namespace OpenEFW
{
	#define BOOST_MI ::boost::multi_index

	template<typename Tag, typename Item> using hash_map2 = ::std::unordered_map<Tag, Item>;

	template<typename Key, typename Item> using hash_map = BOOST_MI::multi_index_container <
		::std::pair<Key, Item>,
		BOOST_MI::indexed_by< 
		BOOST_MI::hashed_unique<BOOST_MI::member<::std::pair<Key, Item>, Key, &::std::pair<Key, Item>::first>>,
		BOOST_MI::sequenced<>
		>>;

	static void test_hash_map() {
		::std::unordered_map<::std::string, int> mm;
		mm.begin()->first;

		using Map = hash_map<::std::string, int>;
		using Pair = ::std::pair<std::string, int>;
		Map hm;
		hm.insert(Pair("ZUI", 341));

		int &sds = hm.begin()->second; // TODO: int& instead of const int

		if (hm.find("ZUI") != hm.end()) ::std::cout << "V: " << hm.begin()->second << ::std::endl;
		
		//for (Map::iterator it = hm.begin(); it != hm.end(); ++it) { ::std:: cout << "V: " << it.get_node()->value << ::std::endl; };
		//hm.find("hi");
	}
};

#else
#include <unordered_map>

namespace OpenEFW
{
	using ::std::unordered_map;
	template<typename Tag, typename Item> using hash_map = unordered_map<Tag, Item>;
};

#endif

#endif