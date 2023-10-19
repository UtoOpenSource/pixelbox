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

#include "base.hpp"
#pragma once

namespace pb {
	
	static constexpr default_port = 14636;

	class AbstractServer : public Abstract { 
		public:
		// NULL for ANY, 0 on success
		virtual int start(const char* address, unsigned short port) = 0;
		virtual void stop() = 0; // stop server
		virtual void tick() = 0; // process server events and all this shit
	};

};


