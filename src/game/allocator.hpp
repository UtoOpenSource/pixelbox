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

};

