#ifndef LUATT_INCLUDE__GENERAL_H
#define LUATT_INCLUDE__GENERAL_H
#include "LuaLib\lua.hpp"
#define LUATT_BUILD "121026"

#include "Defines.h"
#include "Standard.h"

#define lua_boxpointer(L,u) \
	(*(void **)(lua_newuserdata(L, sizeof(void *))) = (u))
#define lua_unboxpointer(L,i)	(*(void **)(lua_touserdata(L, i)))
void *lua_testudata(lua_State *L, int idx, const char *tname);
#endif

#define RENEGADE_VERSION 1037

