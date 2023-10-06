#include "exportlua.h"
#include "raylib.h"
#include <list>

namespace lua {

#define BEGIN(name) {name, [](lua_State* L){
#define END return 0;}},

#define GETS(id) luaL_checkstring(L, id)
#define GETI(id) luaL_checkinteger(L, id)
#define GETN(id) luaL_checknumber(L, id)
#define PUTS(str) lua_pushstring(L, str)
#define PUTN(str) lua_pushnumber(L, str)
#define PUTB(str) lua_pushboolean(L, str)
#define RESULT(i) return i;

static struct luaL_Reg funcs[] = {

BEGIN("SetWindowTitle")
	const char *title = GETS(1);
	SetWindowTitle(title);
END

BEGIN("IsWindowFullscreen")
	PUTB(IsWindowFullscreen());
	return 1;
END

BEGIN("ToggleFullscreen")
	ToggleFullscreen();
END

BEGIN("SetWindowPosition")
	SetWindowPosition(GETI(1), GETI(2));
END

BEGIN("SetWindowSize")
	SetWindowSize(GETI(1), GETI(2));
END

BEGIN("GetScreenWidth")
	PUTN(GetScreenWidth()); return 1;
END

BEGIN("GetScreenHeight")
	PUTN(GetScreenHeight()); return 1;
END

BEGIN("GetMonitorWidth")
	PUTN(GetMonitorWidth(GetCurrentMonitor())); return 1;
END

BEGIN("GetMonitorHeight")
	PUTN(GetMonitorHeight(GetCurrentMonitor())); return 1;
END

BEGIN("GetMonitorRefreshRate")
	PUTN(GetMonitorRefreshRate(GetCurrentMonitor())); return 1;
END

BEGIN("GetWindowPosition")
	Vector2 v = GetWindowPosition();
	PUTN(v.x); PUTN(v.y);
	return 2;
END

BEGIN("ShowCursor")
	ShowCursor();
END

BEGIN("HideCursor")
	HideCursor();
END

BEGIN("IsCursorOnScreen")
	PUTB(IsCursorOnScreen());
	return 1;
END

BEGIN("GetFrameTime")
	PUTN(GetFrameTime());
	return 1;
END

BEGIN("GetTime")
	PUTN(GetTime());
	return 1;
END

BEGIN("SetTargetFPS")
	SetTargetFPS(GETI(1));
END

BEGIN("TakeScreenshot")
	TakeScreenshot(GETS(1));
END

BEGIN("OpenURL")
	OpenURL(GETS(1));
END

// pixelbox's lua is ALREADY patched to support VFS!
// we don't need to provide similar functions here. DRY

#undef BEGIN
#undef END
{nullptr, nullptr}
};

RegisterInitHook h([](lua_State* L) {
	luaL_requiref(L, "raylib", [](lua_State* L) {
		luaL_newlib(L, funcs);
		return 1;
	}, 1);
	lua_pop(L, 1);
	return 0;
});


};
