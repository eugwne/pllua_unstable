#ifndef PLLUA_PGFUNC_H
#define PLLUA_PGFUNC_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <postgres.h>
#include <hstore.h>

int get_pgfunc(lua_State * L);
void register_funcinfo_mt(lua_State * L);


#endif // PLLUA_PGFUNC_H
