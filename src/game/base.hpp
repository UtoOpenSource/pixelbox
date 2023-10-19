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

#include <netinet/in.h>

namespace pb {

	constexpr bool sanity_check_chunk_data = 1;
	using AtomType = uint8_t;

	// we use full range of integers here
	// else will broke chunk size and pixel positions
	static_assert(sizeof(uint16_t) > sizeof(uint8_t));
	// else will break chunk positions
	static_assert(sizeof(uint32_t) > sizeof(uint16_t));
	// else will break hashing and packed chunk position
	static_assert(sizeof(uint64_t) > sizeof(uint32_t));

	struct ChunkPos {
		public:
		union {
			int32_t axis[2];
			uint64_t hash;
		} pack;
		ChunkPos(int32_t x, int32_t y) {
			pack.axis[0] = x; pack.axis[1] = y;
		}
		ChunkPos(const ChunkPos&) = default;
		inline operator uint64_t() const {return pack.hash;}
		inline int32_t  getX() const {return pack.axis[0];}
		inline int32_t  getY() const {return pack.axis[1];}
		inline bool operator==(const ChunkPos& b) const {
			return getX() == b.getX() && getY()==b.getY();
		}
	};

	// important in some cases
	static_assert(sizeof(ChunkPos) == sizeof(uint64_t));

	struct ChunkData {
		public:
		using position_t = int8_t;
		static constexpr uint8_t WIDTH  = 16;
		static constexpr uint8_t HEIGHT = 16;
		static constexpr uint16_t SIZE  = WIDTH*HEIGHT;
		public:
		AtomType atoms[WIDTH*HEIGHT];
		public:
		inline AtomType& get(uint16_t i) {
			if (sanity_check_chunk_data && i >= SIZE)
				throw "index is out of bounds";
			return atoms[i];
		}
		inline AtomType& get(position_t x, position_t y) {
			if (sanity_check_chunk_data && (x >= WIDTH || y >= HEIGHT))
				throw "pixel position is out of bounds";
			return atoms[x + WIDTH*y];
		}
	};

	static_assert(ChunkData::SIZE < UINT16_MAX);

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

	static constexpr uint16_t DATA_SIZE_NET = endian::ton16(ChunkData::SIZE); 

	static constexpr unsigned int DATA_PROTOCOL_ID = (
		(DATA_SIZE_NET << 16) + // chunk size
		(sizeof(AtomType)<< 12) + // pixel type size
		(sizeof(AtomType)<< 8)  + // pixel data size
		0xAF // unique
	);

	// abstract base class... Yay :D
	class Abstract {
		public:
		Abstract(const Abstract&) = delete;
		virtual ~Abstract() = 0;
	};

	/*
	 * Class definitions. All they are likely to be very
	 * abstract or more.
	 */

	// chunks container (and more probably)
	// only IN RAM things, IO processed separately
	// by connection to the server or using the database
	// in Server implementation.
	// also, we're using static polymorphism here :D
	class WorldBase;

	template <class Derived>
	class ConcreteWorld;

	// base classes
	class ClientBase;
	class ServerBase;

	// renderer on top of the container
	class ChunkRenderBase;
};


