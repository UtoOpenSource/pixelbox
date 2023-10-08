#include "exportlua.h"
#include "raylib.h"
#include "assets.h"
#include <list>

namespace lua {

#define BEGIN(name) {name, [](lua_State* L){
#define END return 0;}},

#define GETS(id) luaL_checkstring(L, id)
#define GETI(id) luaL_checkinteger(L, id)
#define GETOPTI(id, def) luaL_optinteger(L, id, def)
#define GETN(id) luaL_checknumber(L, id)
#define GETOPTN(id, def) luaL_optnumber(L, id, def)
#define PUTS(str) lua_pushstring(L, str)
#define PUTN(str) lua_pushnumber(L, str)
#define PUTI(str) lua_pushnumber(L, (int)str)
#define PUTB(str) lua_pushboolean(L, str)
#define RESULT(i) return i;

static Color luaL_checkcolor(lua_State* L, int idx) {
	luaL_checktype(L, idx, LUA_TTABLE);
	Color c = WHITE;
	for (int i = 1; i <= 4; i++) {
		if (lua_rawgeti(L, idx, i) == LUA_TNUMBER) {
			// FIXME : may be mot very accurate... but who cates :D
			((unsigned char*)&c)[i-1] = lua_tointeger(L, -1); 
		}
		lua_pop(L, -1);
	}
	return c;
}
#define GETCOL(id) luaL_checkcolor(L, id)

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

// TODO : directory API may be wonderful, but raylib DOES NOT
// support custom directory callbacks, so... this should be
// handwritten there... Later :p

// TODO : do we really need compression API here???

BEGIN("IsKeyPressed")
	PUTB(IsKeyPressed(GETI(1)));
	return 1;
END

BEGIN("IsKeyReleased")
	PUTB(IsKeyReleased(GETI(1)));
	return 1;
END

BEGIN("IsKeyDown")
	PUTB(IsKeyDown(GETI(1)));
	return 1;
END

// WARNING! unlike regular raylib functions, theese may return nil!
BEGIN("GetKeyPressed")
	int i = GetKeyPressed();
	PUTI(i);
	return int(i != 0);
END

BEGIN("GetCharPressed")
	int i = GetCharPressed();
	PUTI(i);
	return int(i != 0);
END

// MOUSE

BEGIN("IsMouseButtonPressed")
	PUTB(IsMouseButtonPressed(GETI(1)));
	return 1;
END

BEGIN("IsMouseButtonReleased")
	PUTB(IsMouseButtonReleased(GETI(1)));
	return 1;
END

BEGIN("IsMouseButtonDown")
	PUTB(IsMouseButtonDown(GETI(1)));
	return 1;
END

BEGIN("GetMousePosition")
	Vector2 v = GetMousePosition();
	PUTN(v.x); PUTN(v.y);
	return 2;	
END

BEGIN("SetMousePosition")
	SetMousePosition(GETI(1), GETI(2));
END

BEGIN("GetMouseDelta")
	Vector2 v = GetMouseDelta();
	PUTN(v.x); PUTN(v.y);
	return 2;	
END

BEGIN("GetMouseWheelMove")
	PUTN(GetMouseWheelMove());
	return 1;
END

BEGIN("GetTouchPosition")
	Vector2 v = GetTouchPosition(GETI(1));
	PUTN(v.x); PUTN(v.y);
	return 2;	
END

BEGIN("GetTouchPointId")
	PUTI(GetTouchPointId(GETI(1)));
	return 1;
END

BEGIN("GetTouchPointCount")
	PUTI(GetTouchPointCount());
	return 1;
END

// Drawing (Finnaly)
// Uses global color variable!

#define GETF(i) float(GETN(i))

BEGIN("DrawLine")
	Vector2 v1 = {GETF(1), GETF(2)};
	Vector2 v2 = {GETF(3), GETF(4)};

	DrawLineEx(v1, v2,
		GETOPTN(5, 1.0f),
		GETCOL(6)
	);
END

BEGIN("DrawRectangle")
	DrawRectangle(
		GETF(1), GETF(2), GETF(3), GETF(4),
		GETCOL(5)
	);
END

BEGIN("DrawRectangleLinesEx")
	DrawRectangleLinesEx(
		Rectangle{GETF(1), GETF(2), GETF(3), GETF(4)},
		float(GETOPTN(5, 1.0f)), GETCOL(6)
	);
END

BEGIN("DrawRectangleGradientEx")
	DrawRectangleGradientEx(
		Rectangle{GETF(1), GETF(2), GETF(3), GETF(4)},
		GETCOL(5), GETCOL(6), GETCOL(7), GETCOL(8)
	);
END

// rotation here is at the end!
BEGIN("DrawTextureEx")
	DrawTextureEx(
		GetTextureAsset(LookupAssetID(GETS(1))),
		Vector2{GETF(2), GETF(3)},
		GETOPTN(6, 0.0), GETF(4), GETCOL(5)
	);
END

// color here is BEFORE origin and rotation!
BEGIN("DrawTexturePro")
	DrawTexturePro(
		GetTextureAsset(LookupAssetID(GETS(1))),
		Rectangle{GETF(2), GETF(3), GETF(4), GETF(5)},
		Rectangle{GETF(6), GETF(7), GETF(8), GETF(9)},
		Vector2{float(GETOPTN(11, 0.0)), float(GETOPTN(12, 0.0))}, 
		GETOPTN(13, 0.0), GETCOL(10)
	);
END

// spacing here is going AFTER the color!
BEGIN("DrawText")
	DrawTextEx(
		GetFontDefault(), 
		GETS(1), 
		Vector2{GETF(2), GETF(3)},
		GETF(4), GETOPTN(6, 1),
		GETCOL(5)
	);
END

// get line bounds size

BEGIN("MeasureTextEx")
	Vector2 r = MeasureTextEx(
		GetFontDefault(), 
		GETS(1),
		GETF(2),
		GETOPTN(6, 1)
	);	 // Measure string size for Font
	PUTN(r.x); PUTN(r.y);
	return 2;
END

// extra funcs

BEGIN("PlaySound")
	PlayAssetSound(LookupAssetID(GETS(1)), GETOPTN(2, 1.0), GETOPTN(3, 1.0));
END

BEGIN("SetMasterVolume")
	SetMasterVolume(GETF(1));
END

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
