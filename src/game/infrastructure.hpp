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
#include "base.hpp"
#include <functional>
#include <memory>
#include "plalloc.h"
#include "assert.h"
#include "robin_map.h"

namespace pb {

	template <typename T>
	class PLAlloctor {
		struct Holder {
			::plalloc_t ctx = nullptr;
			Holder() {
				ctx = plalloc_initialize(sizeof(T), 128);
				if (!ctx) throw "OOM";
			}
			~Holder() {
				plalloc_uninitialize(ctx);
			}
		};
		::std::shared_ptr<Holder> h; // all copying is handled here
		public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    PLAlloctor(){
			h = ::std::make_shared<Holder>();
		}

		// now we satisfy std::allocator criteria
		bool operator==(const PLAlloctor& s) {
			return h.get() == s.h.get();
		}
    ~PLAlloctor() {} // handled by shared pointer

    pointer address(reference x) const {return &x;}
    const_pointer address(const_reference x) const {return &x;}
    size_type max_size() const throw() {
			return 1;
		}

    pointer allocate(size_type n, const void* h__ = nullptr) {
			return static_cast<pointer>(plalloc_alloc(h->ctx));
    }

    void deallocate(pointer p, size_type n = 0) {
			if (n != 1) throw "oh no";
			plalloc_free(h->ctx, p);
    }
	};

	/*
	template <typename T>
	struct HashAccessor {
		using key_type = void;
		// your exemplar must implement THIS!
		//uint64_t get_hash(T*);
		//T*       get_next(T*);
		//void     set_next(T*);
		//key_type get_key(T*); 
	};

	// can't allocate/destroy objects! can only keep them inside
	template <typename T, typename HACS = HashAccessor<T> >
	class HashMapImpl {
		T** map = nullptr;
		int    loglen   = -1; // log(len) of buckets array
		size_t hashmask =  0;
		size_t count    =  0; // count of buckets
		int    minlen   =  6; // 64 is the minimum
		private:
		// mask is (pow2len-1)
			// -n is length 0
			//  0 is length 1
			//  1 is length 2
			//  2 is length 4
			//  3 is length 8
			//  ...
		static size_t getlen(int loglen) {
			return (loglen>=0) ? (1 << (loglen)) : 0; // just extra checks
		}
		static int    logof(size_t len) {
			if ((len & (len-1)) != 0) throw "length is not power of 2!";
			if (len < 0) return -1; // empty

			int bit;
			for (bit = 0; bit < (2*8); bit++) { // you will never need more
				if (len & (1 << (bit))) return bit; 
			}
			throw "length is too big!";
		}
		public:
		size_t max_buckets() const { // maximum buckets count
			return getlen(2*8);
		}
		size_t buckets_count() const { // count of buckets :)
			return getlen(loglen);
		}
		class iterator {
			friend class HashMapImpl;
			private:
			HashMapImpl* p = nullptr;
			size_t i = 0;
			T* n = nullptr;
			void next() {
				if (!p->map) {
					n = nullptr;
					return; // map is empty!
				}
				if (n) n = HACS::get_next(n); // get next item
				// get first item in next bucket if end of current bucket
				for (size_t o = i; !n && o < getlen(p->loglen); o++)
					n = p->map[o];
				// sets nullptr => end()
			}
			public:
			explicit iterator(HashMapImpl* _p, size_t _i = 0) {
				p = _p;
				i = _i;
				n = nullptr;
				next(); // hehe
			}
			iterator(const iterator&) = default; // hehehe
			iterator& operator++() {
				next();
				return *this;
			}
			iterator operator++(int) {
				iterator retval = *this;
				++(*this);
				return retval;
			}
			bool operator==(iterator other) const {
				return n == other.n;
			}
			bool operator!=(iterator other) const {
				return !(*this == other);
			}
			T& operator*() const {
				return n;
			}
			T* operator->() const {
				return n;
			}
		};
		public:
		using key_type = typename HACS::key_type; 
		using mapped_type = T; 
		using value_type  = T; 
		using size_type   = size_t;
		using difference_type = ptrdiff_t;
		using hasher      = HACS;
		using reference   = value_type&;
		using const_reference   = const value_type&;
		using pointer   = value_type*;
		using const_pointer   = const value_type*;
		public:
		HashMapImpl() {}
		HashMapImpl(const HashMapImpl&) = delete; // to make things simple
		HashMapImpl& operator=(const HashMapImpl&) = delete; // too

		// does not checks is item inserted already!
		void insert(T* item) {
			if (!map) throw "map is empty!"; // should rehash first!
			if (!item) return;
			if (HACS::get_next(item))
				throw "item is already inserted somewhere!";

			size_t h = HACS::get_hash(item);
			h = h & hashmask;
			T* node = map[h];
			// insert
			HACS::set_next(item, node);
			map[h] = item;
		}

		// try to find exact same pointer
		T* find(T* item) {
			if (!item) return nullptr;
			if (!map) return nullptr; // ok
		
			size_t h = HACS::get_hash(item);
			h = h & hashmask;
			T* node = map[h];

			while (node) {
				if (node == item) return node; // pointer compare...

			}
		}

		T* find(HACS::key_type key) {

		}

		void rehash(size_t s) {
			int newlog = logof(s);
			size_t newsize = getlen(newlog);
			assert(s == newsize);	// important	

			if (!s) { // clear
				delete[] map; // nice
				map = nullptr;
				loglen = 0;
				count  = 0;
			} else {
				T** old = map; 
				map = new T[s]; // must throw on error

				// reinsert

				loglen = 0;
			}
		}

		void clear() {
			
		}

		~HashMapImpl() {

		}

		iterator begin() {
			return iterator(this);
		}
		iterator end() {
			return iterator(this, 1<<this->loglen);
		}
		
	}; */

};

