/*
 * This file is a part of Pixelbox - Infinite 2D sandbox game
 * Copyright (C) 2023 UtoECat
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <https://www.gnu.org/licenses/>
 */

#include "doctest.h"
#include "infrastructure.hpp"
#include <memory>
#include <array>

struct Item {
	int a;
	Item(int b) : a(b) {}
	~Item() {};
};

using ALLOC  = pb::PLAlloctor<Item>;
using TRAITS = std::allocator_traits<ALLOC>;

TEST_CASE("plalloc-basic") {
	ALLOC A;

	CHECK(A.max_size() == 1);
	CHECK(TRAITS::max_size(A) == 1);

	Item* p = A.allocate(1);
	CHECK(p != nullptr);
	A.deallocate(p);

	std::array<Item*, 512> array;
	for (size_t i = 0; i < array.max_size(); i++) {
		array[i] = A.allocate(1);
		CHECK(array[i] != nullptr);
		TRAITS::construct<Item>(A, array[i], i);
	}

	for (size_t i = 0; i < array.max_size(); i += 4) {
		A.deallocate(array[i]);
		array[i] = nullptr;
	}

	for (size_t i = 0; i < array.max_size(); i += 4) {
		if (array[i]) continue;
		array[i] = A.allocate(1);
		CHECK(array[i] != nullptr);
		TRAITS::construct<Item>(A, array[i], i + array.max_size());
	}

	for (size_t i = 0; i < array.max_size(); i++) {
		A.deallocate(array[i]);
		array[i] = nullptr;
	}
}

#include <list>
#include <typeinfo>
#include <stdio.h>

template <typename T>
using AllocWrap = pb::PLAlloctor<T>; 

struct Pot {
	char val;
};

TEST_CASE("plalloc-copy") {
	AllocWrap<Item> A;
	Item *a = A.allocate(1);
	AllocWrap<Pot> B(A);
	Pot* p = B.allocate(1);
	CHECK(A.count() == 2);
	A.deallocate(a);
	B.deallocate(p);
}

TEST_CASE("plalloc-stl") {
	AllocWrap<Item> A;

	auto list = std::list<Item, AllocWrap<Item>>(A);
	//std::list<Item, pb::PLAlloctor<Item>> list;

	list.push_front(10);
	list.push_front(20);

	CHECK(list.size() == 2);

	auto list2 = list; // + 2
	list2.push_front(30);

	CHECK(list.get_allocator().count() == (list.size() + list2.size()));
}

#include <stdint.h>


static inline unsigned rol(unsigned r, int k) {
	return (r << k) | (r >> (32 - k));
}

static uint64_t murmur(const char* str) {
	uint64_t h = 3323198485ul;
	for (; *str; ++str) {
		h ^= *str;
		h *= 0x5bd1e995;
		h ^= h >> 15;
	}

	return h;
}

#include <string>

struct Profile {
	std::string name;
	Profile* next;
	uint64_t hash;
	public:
	Profile(const char* n) {
		name = n;
		hash = murmur(name.c_str());
	}
};

struct HashAccessorProfile {
	using key_type = std::string;
	static constexpr float hash_policy = 2.9;
	static uint64_t get_hash(Profile* p) {
		return p->hash;
	}
	static Profile* get_next(Profile* p) {
		return p->next;
	}
	static void     set_next(Profile* p, Profile* n) {
		p->next = n;
	}
	static key_type get_key(Profile* p) {
		return p->name;
	}
	static uint64_t hash_key(key_type k) {
		return murmur(k.c_str());
	}
};

using ProfTraits = typename ::std::allocator_traits<AllocWrap<Profile>>;

static char incchar(char &v) {
	v++;
	if (v > 'z') {
		v = 'a';
		return true;
	}
	return false;
}

using Cont = pb::HashContainer<Profile, HashAccessorProfile>;

TEST_CASE("hashcontainer") {
	Cont map;
	AllocWrap<Profile> alloc;

	Profile* p = alloc.allocate(1);
	CHECK(p != nullptr);
	alloc.deallocate(p);

	CHECK(Cont::getlen(-1) == 0);
	CHECK(Cont::getlen(0)  == 1);
	CHECK(Cont::getlen(2)  == 4);
	CHECK(Cont::getlen(3)  == 8);

	CHECK(Cont::logof(0) == -1);
	CHECK(Cont::logof(1) == 0);
	CHECK(Cont::logof(4) == 2);
	CHECK(Cont::logof(8) == 3);
	CHECK(32 == Cont::getlen(Cont::logof(32)));

	CHECK(map.buckets_count() == 0);
	map.rehash(128);
	CHECK(map.buckets_count() == 128);
	map.clear([](Profile*){});
	CHECK(map.buckets_count() == 0);

	p = alloc.allocate(1);
	CHECK(p != nullptr);
	ProfTraits::construct(alloc, p, "zaeba");
	CHECK(map.insert(p) == true);

	CHECK(p->name == "zaeba");
	CHECK(p->hash == murmur("zaeba"));

	CHECK(map.buckets_count() > 1);
	CHECK(map.find("zaeba") != nullptr);

	p = alloc.allocate(1);
	CHECK(p != nullptr);
	ProfTraits::construct(alloc, p, "aboba");
	CHECK(map.insert(p) == true);

	CHECK(map.begin() != map.end());
	
	for (auto& node : map) {
		printf("%p, %s\n", &node, node.name.c_str());
	}

	map.rehash(128);
	CHECK(map.find("zaeba") != nullptr);

	map.clear([&alloc](Profile* p){
		alloc.deallocate(p);
	});
}

TEST_CASE("hashcontainer-insert") {
	Cont map;
	AllocWrap<Profile> alloc;

	char rand[8] = "aaaaaaa";
	for (int i = 0; i < 1024; i++) {
		Profile* p = alloc.allocate(1);
		CHECK(p != nullptr);
		ProfTraits::construct(alloc, p, rand);
		CHECK(map.insert(p));
		for (int j = 0; j < 7 && incchar(rand[j]); j++) {}
	}

	CHECK(map.find("azaaaaa") != nullptr);

	Profile* p = map.find("zgaaaaa");
	CHECK(p != nullptr);
	map.remove(p);
	alloc.deallocate(p);
	CHECK(map.find("zgaaaaa") == nullptr);

	int cnt = 0;
	for (auto a = map.begin(); a != map.end();) {
		if (cnt++) {
			alloc.deallocate(a.remove());
		} else a++;
		if (cnt > 3) cnt = 0;
	}

	for (auto& node : map) {
		if (node.name.c_str()[0] == 'z')
			printf("%p, %s\n", &node, node.name.c_str());
	}

	map.clear([&alloc](Profile* p){
		alloc.deallocate(p);
	});

	CHECK(map.buckets_count() == 0);
}


