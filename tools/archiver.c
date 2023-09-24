#define LUA_IMPL
#include "minilua.h"
#include <stdio.h>
#include <raylib.h>

static int lua_deflate(lua_State* L) {
	int outsize = 0;
	size_t insize = 0;
	const char* str = luaL_checklstring(L, 1, &insize);
	char* res = CompressData(str, insize, &outsize);
	if (!res) return 0;
	lua_pushlstring(L, res, outsize);
	MemFree(res);
	return 1;
}

int main(int argc, const char** argv) {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	SetTraceLogLevel(LOG_NONE);
	lua_pushcfunction(L, lua_deflate);
	lua_setglobal(L, "compress");

	lua_newtable(L);
	for(int i = 1; i < argc; i++) {
		lua_pushstring(L, argv[i]);
		lua_seti(L, -2, i);
	}
	lua_setglobal(L, "arg");

	if (luaL_dofile(L, "tools/ar.lua") != LUA_OK) {
		fprintf(stderr, "%s", lua_tostring(L, -1));
		return -1;
	}

	lua_close(L);
	return 0;
}
