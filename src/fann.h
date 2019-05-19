/******************************************************************
	LuaFann: Lua bindings for the Fast Artificial Neural Network Library
    Copyright (C) 2008-2009  Werner Stoop

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************************************************************/

/*
 *	This module forms the FANN wrapper for Lua
 */

#include <lua.h>
#include <lauxlib.h>


LUALIB_API int luaopen_fann(lua_State *L);
#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
LUALIB_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);
#endif
