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
#include <stdint.h>
#include <limits.h>
#include <atomic>

namespace pb {

	// we use full range of integers here
	// else will broke chunk size and pixel positions
	static_assert(sizeof(uint16_t) > sizeof(uint8_t));
	// else will break chunk positions
	static_assert(sizeof(uint32_t) > sizeof(uint16_t));
	// else will break hashing and packed chunk position
	static_assert(sizeof(uint64_t) > sizeof(uint32_t));

	// https://stackoverflow.com/questions/1583791/constexpr-and-endianness
	// implementation defined, but works well on GCC and CLANG
	// used also in net packet building/parsing in both client and server
	// TODO: add C++20 case with #include <endian>
	namespace endian {
    namespace I {
			//41 42 43 44 = 'ABCD' hex ASCII code
			static constexpr uint32_t LITTLE{ 0x41424344u };
			//44 43 42 41 = 'DCBA' hex ASCII code
			static constexpr uint32_t BIG{ 0x44434241u };
			static constexpr uint32_t NATIVE{ 'ABCD' };
		}

    //Compare
    static constexpr bool is_little = I::NATIVE == I::LITTLE;
    static constexpr bool is_big    = I::NATIVE == I::BIG;

		static_assert(I::NATIVE == I::BIG || I::NATIVE == I::LITTLE, 
			"Can't detect endianess at compile time!"
		);

		constexpr uint16_t ton16(uint16_t n) { // to net endian (big)
			if constexpr (is_little) {
				return ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
			}
			return n;
		}
		constexpr uint16_t toh16(uint16_t n) { // to host endian
			if constexpr (is_little) { // need swap
				return ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
			}
			return n; // pass
		}
	};

	class Default {
		public:
		Default() = default;
		Default(const Default&) = delete;
		Default& operator=(const Default&) = delete;
	};

	// abstract base class... Yay :D
	class Abstract : public Default {
		public:
		virtual ~Abstract() = 0;
	};

	// lowlevel
	// when release() returns true, you must destroy this object properly
	// and free memory
	// in pixelbox some objects are created using custom linear allocator, therefore
	// they cannot be delete'd, you must call special method in allocator
	// to do so
	class Shared {
		private:
		uint32_t refcnt = 0;
		public:
		void aqquire() {
			refcnt++;
		}
		bool release() {
			if (!refcnt) throw "bad reference count!";
			refcnt--;
			if (!refcnt) return false;
			return true;
		}
	};

	class SpinLock {
		std::atomic_bool lo;
		public:
		bool try_lock() {
			bool unlocked = false;
			return lo.compare_exchange_weak(
				unlocked, true, std::memory_order_acquire
			);
		}
		void lock() {
			while(!try_lock()) {
				;
			}
		}
		void unlock() {
			lo.store(false, std::memory_order_release);
		}
	};


};


