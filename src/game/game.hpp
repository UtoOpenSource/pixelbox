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
#include "chunk.hpp"

namespace pb {

	static constexpr unsigned int DATA_PROTOCOL_ID = (
		(DATA_SIZE_NET << 16) + // chunk size
		(sizeof(AtomType)<< 12) + // pixel type size
		(sizeof(AtomType)<< 8)  + // pixel data size
		0xAF // unique
	);

	static const char VERSION_MAJOR = 0;
	static const char VERSION_MINOR = 7;
	static const char VERSION_PATCH = 0;
	static const char VERSION_ID = 
		VERSION_MAJOR*100 + VERSION_MINOR*10 + VERSION_PATCH;

	class Service : public Default {
		public:
		virtual ~Service() = 0;
		virtual void open(const char*, unsigned short) = 0; // ip(client) or database(server)
		virtual void close() = 0; // closing server will bock for a big time...
		virtual bool status() = 0; // is running
		virtual const char* get_error() = 0; // get error message

		virtual void auth(const char* cli_key) = 0; // no need to be called on server yet?

		virtual void update() = 0; // network + world
		virtual void collect() = 0; // collect garbage
		public: // raw recieve data again (from db or server)
		public: // generic cache loookups
		Entity* get_entity(EntityID);
	};

};


