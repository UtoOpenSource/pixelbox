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
#include "allocator.hpp"
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
