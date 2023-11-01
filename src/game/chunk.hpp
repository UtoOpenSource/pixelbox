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
#include "base.hpp"

namespace pb {

	constexpr bool sanity_check_chunk_data = 1;
	struct alignas(2) AtomType {
		uint8_t type;
		uint8_t data;
		public:
		inline operator uint8_t() const {
			return type;
		}
		inline auto operator==(const AtomType& b) const {
			return type == b.type;
		}
		inline auto operator!=(const AtomType& b) const {
			return type != b.type;
		}
	};

	static_assert(sizeof(AtomType) == 2, "your compiler is sick");

	/**
	 * @description position of the chunk
	 */
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
		inline bool operator!=(const ChunkPos& b) const {
			return !(*this == b);
		}
	};

	// important in some cases
	static_assert(sizeof(ChunkPos) == sizeof(uint64_t));

	/**
	 * @description chunk data, used in both server and client
	 */
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

	static constexpr uint16_t DATA_SIZE_NET = endian::ton16(ChunkData::SIZE); 

	static constexpr unsigned int DATA_PROTOCOL_ID = (
		(DATA_SIZE_NET << 16) + // chunk size
		(sizeof(AtomType)<< 12) + // pixel type size
		(sizeof(AtomType)<< 8)  + // pixel data size
		0xAF // unique
	);

	struct ChunkFlags {
		bool is_ready    : 1;
		bool is_newgen   : 1;
		bool is_colleted : 1; // remove this chunk from everything
		bool is_updating : 1;
		bool is_playing  : 1;
	};

	static constexpr uint8_t DEFAULT_USAGE = 20;

	class BaseChunk : public Default, Shared {
		public: // info
		ChunkPos position;
		uint8_t usage_factor = DEFAULT_USAGE;
		ChunkFlags flags;
		public: // data
		ChunkData data[2];
		bool      index;
		public: 
		BaseChunk(ChunkPos p) : position(p) {};
		~BaseChunk(){}
	};

};


