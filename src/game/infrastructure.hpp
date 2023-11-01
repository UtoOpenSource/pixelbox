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
#include <type_traits>

namespace pb {

	template <typename T>
	class PLAlloctor {
		public:
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

		private:
		// holy magic
		// reinterpret cast std::shared_ptr<T> in such a way, 
		// that new pointer<U> will maintain the same reference info as
		// original shared_ptr<T>, but pointer will be not the same

		template<class Ta, class U>
		static std::shared_ptr<Ta> __rpc(const std::shared_ptr<U>& r) noexcept {
    	auto p = reinterpret_cast<typename std::shared_ptr<Ta>::element_type*>(r.get());
    	return std::shared_ptr<Ta>{r, p};
		}

		public:

		template<class U>
		PLAlloctor(const PLAlloctor<U>& other) noexcept {
			// can use the same allocator :)
			if constexpr(sizeof(U) >= sizeof(T)) {
				// siletnly convert
				h = __rpc<Holder, typename PLAlloctor<U>::Holder>(other.h);	
			} else { // fuck, create new one
				h = ::std::make_shared<Holder>();
			}
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
			(void)h__;
			if (n < 1) return nullptr;
			return static_cast<pointer>(plalloc_alloc(h->ctx));
    }

    void deallocate(pointer p, size_type n = 0) {
			if (n > 1) throw "oh no";
			plalloc_free(h->ctx, p);
    }

		size_t count() {
			return plalloc_count(h->ctx);
		}

	};

	// hash container configurator
	template <typename T>
	struct HashAccessor {
		// key type
		using key_type = void;

		// if (buckets_count() * hash_policy < count()) do_rehash(good_size(count()));
		// if it less than 1, rehash will be NEVER performed!
		static constexpr float hash_policy = 2.9;

		// your instances must implement ALL OF THIS!
		//uint64_t get_hash(T*);
		//T*       get_next(T*);
		//void     set_next(T*, T*);
		//key_type get_key(T*); 
		//uint64_t hash_key(key_type k);
	};

	// can't allocate objects! can only keep them inside.
	// gigaconfigurable hash container with reference consistency(until node is deleted)
	// and extreme flexibility trough the HashAccessor interface.
	// (Warning: whire references are ok, iterator will still be invalidated after the
	// rehash!)
	//
	// No need for separate node objects : you specify where is key/hash/next node
	// values are located inside of your object.
	//
	// Written fully by me (UtoECat), from scratch
	//
	// Warning: this is NOT full replacement for hashmaps, this is a CORE of the hashmap
	// classes and components.
	// Because of that, you must explicitly destroy objects after removal and 
	// explicitly allocate them, and also you mustdestroy them all before container
	// destruction, in other case you will likely have memory leak.
	// But if you have linear allocator, this might be not the case.
	template <typename T, typename HACS = HashAccessor<T> >
	class HashContainer : public Default {
		T** map = nullptr;
		int    loglen   = -1; // log(len) of buckets array
		static constexpr int minlog   =  6; // 64 is the minimum
		static constexpr float hash_policy = HACS::hash_policy > 1 ? HACS::hash_policy : 0;
		// capacity
		size_t icount = 0; // count of items
		size_t tcount = minlog * hash_policy; // target count of items
		public:
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
		size_t hashmask() const {
			return (1<<getlen(loglen))-1;
		}
		static int    logof(size_t len) {
			if (len == 0) return -1;
			if ((len & (len-1)) != 0)
				throw "length is not power of 2!";
			if (len < 0) return -1; // empty

			int bit;
			for (bit = 0; bit < (2*8); bit++) { // you will never need more
				if (len & (1 << (bit))) return bit; 
			}
			throw "lengthContainer big!";
		}
		public:
		size_t max_buckets() const { // maximum buckets count
			return getlen(2*8);
		}
		size_t buckets_count() const { // count of buckets :)
			return getlen(loglen);
		}
		class iterator {
			friend class HashContainer;
			private:
			HashContainer* c = nullptr;
			size_t i = 0;
			T* n = nullptr, *p = nullptr; // node and previous node
			void next() {
				if (!c->map) {
					n = nullptr;
					p = nullptr;
					return; // map is empty!
				}

 				// get next item
				p = n;
				if (n) n = HACS::get_next(n);
				if (!n) { // end of the bucket
					p = nullptr;
					// get first item in next bucket
					for (; !n && i < c->buckets_count(); i++)
						n = c->map[i];
				}
				// sets nullptr => end()
			}

			public:
			// removes current node and returns it
			T* remove() {
				if (!n) return nullptr; 
				T* res = n;
				n = c->remove(n); // returns next node
				
				// end of the bucket. need to fix that
				if (!n)
					next();
				
				return res; // return current node, btw :D
			}
			explicit iterator(HashContainer* _c, size_t _i = 0) {
				c = _c;
				i = _i;
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
			T& operator*() {
				return *n;
			}
			T* operator->() const {
				return n;
			}

			// small workaround to validate iterator after raw_insert()
			// or rehash(). Please keep in mind, that this is basicly find()
			// item again in the hashtable, aka may be a bit slower than you except.
			// call this ONLY when it's important (actual insert or rehash is happened)
			void validate() {
				if (!c->map || !n)
					return;

				size_t hash = HACS::get_hash(n);
				hash = hash & c->hashmask();
				i = hash;
				p = c->find_previous(n);
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
		HashContainer() {}
		HashContainer(const HashContainer&) = delete; // to make things simple
		HashContainer operator=(const HashContainer&) = delete; // too

		// does rehash if conditions are met
		bool optional_rehash() {
			if (!map)
				return rehash(1);

			if constexpr (hash_policy >= 1) {
				if (icount >= buckets_count() * hash_policy) {
					size_t next_size = buckets_count() << 1;
					if (next_size < max_buckets())
						return rehash(next_size);
				}
			}
			return false;
		}

		// does not checks is item inserted already! and do not rehashes
		// when inserted and iterator is placed at the start of bucket, iterator will be
		// invalidated : remove() on it will NOT work properly and my caouse memory leak!
		// do not insert in your iterator loops at all - this is the solution
		void raw_insert(T* item) {
			if (!map) throw "map is empty!"; // should rehash first!
			if (!item) return;
			if (HACS::get_next(item))
				throw "item is already inserted somewhere!";

			size_t h = HACS::get_hash(item);
			h = h & hashmask();
			T* node = map[h];
			// insert
			HACS::set_next(item, node);
			map[h] = item;

			// success
			icount++;
		}

		// checks if the item exists and does the rehash
		// both inserts may invalidate iterator in different ways!
		bool insert(T* item) {
			optional_rehash();
			T* x = find(item);
			if (!x) {
				raw_insert(item);
				return true;
			}
			return false;
		}

		// try to find exact same pointer
		T* find(T* item) const {
			if (!item) return nullptr;
			if (!map) return nullptr; // ok
		
			size_t h = HACS::get_hash(item);
			h = h & hashmask();
			T* node = map[h];

			while (node) {
				if (node == item) return node; // pointer compare...
				node = HACS::get_next(node);
			}
			return nullptr;
		}

		T* find(key_type key) const {
			if (!map) return nullptr; // ok
		
			size_t hash = HACS::hash_key(key);
			size_t h = hash & hashmask();
			T* node = map[h];

			while (node) {
				if (HACS::get_hash(node) == hash) return node; // pointer compare...
				node = HACS::get_next(node);
			}
			return nullptr;
		}

		// finds previous node for the current one
		// this operation is slow and exists only for specific 
		// iterator validation cases (that's a bad thing anyways!)
		T* find_previous(T* item) const {
			if (!item) return nullptr;
			if (!map) return nullptr; // ok
		
			size_t h = HACS::get_hash(item);
			h = h & hashmask();
			T* node = map[h], *prev = nullptr;

			while (node) {
				if (node == item) return prev; // pointer compare...
				prev = node;
				node = HACS::get_next(node);
			}
			return nullptr;
		}

		T* remove(T* item) { // returns next node
			if (!item) return nullptr;
			if (!map) return nullptr; // ok
		
			size_t h = HACS::get_hash(item);
			h = h & hashmask();
			T* node = map[h], *prev = nullptr;

			while (node) {
				T* next = HACS::get_next(node);
				if (node == item) { // pointer compare...
					// remove
					if (prev) HACS::set_next(prev, next);
					else map[h] = next;
					icount--;

					// return
					HACS::set_next(node, nullptr);
					return next; // next node .O.
				}
				prev = node;
				node = next;
			}
			throw "node is not exists in the hashtable!";
			return nullptr;
		}
	
		// this thing does not destroy objects! provide your destructor as first arg
		void clear(std::function<void(T*)> destructor) {
			if (!map) return;

			for (int64_t i = 0; i < buckets_count(); i++) {
				T* node = map[i];

				while (node) {
					T* f = node;
					node = HACS::get_next(node);
					assert(node != f && "fuck");
					if (destructor) {
						destructor(f);
					}
				}
				map[i] = nullptr;
			}

			delete[] map; // nice
			map = nullptr;
			loglen = -1;
			icount = 0;
			tcount = minlog * hash_policy;
		}

		bool rehash(size_t s) {
			if (!s) {
				clear({});
				return false;
			} 

			int newlog = logof(s);
			if (newlog < minlog) newlog = minlog;
			size_t newsize = getlen(newlog);

			{
				T** old = map; 
				map = new T*[newsize]; // must throw on error
				size_t oldlen = getlen(loglen);
				loglen = newlog; // or this will corrupt map
				size_t oldcount = icount;
				icount = 0; // we should get the same count as before :)
				tcount = newsize * 2.9; // hash policy

				for (int64_t i = 0; i < buckets_count(); i++) {
					map[i] = nullptr;
				}
				if (!old) return true; // done

				// reinsert
				for (int64_t i = 0; i < oldlen; i++) {
					T* node = old[i], *next = nullptr;

					while (node) {
						next = HACS::get_next(node);
						HACS::set_next(node, nullptr); // simulate removal
						raw_insert(node);
						node = next;
					}
				}
				assert(icount == oldcount);

				delete[] old; // leeets go
				return true;
			}
		}

		~HashContainer() {
			clear([](T*){});
		}

		iterator begin() {
			return iterator(this);
		}
		iterator end() {
			return iterator(this, max_buckets());
		}
	
		size_t count() {
			return map ? icount : 0;
		}

		operator bool() {
			return count();
		}

		T** raw() {
			return map;
		}

	};

};

