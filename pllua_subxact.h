#ifndef PLLUA_SUBXACT_H
#define PLLUA_SUBXACT_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <postgres.h>

int use_subtransaction(lua_State * L);

#endif // PLLUA_SUBXACT_H
