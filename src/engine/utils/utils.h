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

#pragma once
#include <functional>
#include <string>
#include <stdint.h>

namespace util {

static constexpr HASH_SIZE = 128;

template <typename T>
struct Hashable {
	// you must implement theese functions, and base your class on this
	uint64_t  hash() const;
	Hashable* next();
	void      set_next(Hashable*);
	T*
};

template <typename T> // T must be a class based on Hashable! (or similar)
class HashTable {
	T**  table; // array of pointers on nodes
	char logsize;
	public:
	HashTable();
	HashTable(const HashTable&) = delete;
	HashTable& operator=(const HashTable&) = delete;
	~HashTable();
	public:
	void insert(T* item);
	void remove(T* item);
	bool find  (T* item);
	T*   find  (uint64_t hash);
};

};
