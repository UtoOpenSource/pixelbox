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

#include "exportlua.h"
#include <list>
#include "vfs.h"
#include <string.h>

namespace lua {

	static std::list<lua_CFunction> *hooklist = nullptr;

	RegisterInitHook::RegisterInitHook(lua_CFunction fun) {
		if (!fun) throw "bad hook to register!";
		if (!hooklist) hooklist = new std::list<lua_CFunction>();
		hooklist->push_back(fun);
	}
	
	void applyRegisteredHooks(lua_State* L) {
		if (!hooklist) return;
		for (auto& i : *hooklist) {
			lua_pushcfunction(L, i);
			if (lua_pcall(L, 0, 1, -1) != LUA_OK) {
				vflog("LUA: APPLYREG: %s", luaL_tolstring(L, -1, nullptr));
				throw "fatal error! Init hook failed!";
			};
			lua_pop(L, 1);
		}
	}

	static int db_traceback(lua_State *L) {
		const char *msg = lua_tostring(L, 1);
		if (msg == NULL && !lua_isnoneornil(L, 1)) /* non-string 'msg'? */
			lua_pushvalue(L, 1);			/* return it untouched */
		else {
			int level = (int)luaL_optinteger(L, 2, 1);
			luaL_traceback(L, L, msg, level);
		}
		return 1;
	}

	lua_State* newState() {
		lua_State* L = luaL_newstate();
		luaL_openlibs(L);
		applyRegisteredHooks(L); // theese are CALLED!
		return L;
	}

	void unregisterHooks() {
		delete hooklist;
	}

};
