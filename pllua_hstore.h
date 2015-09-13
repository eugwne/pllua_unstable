/*
 * pllua hstore type wrapper
 * Author: Eugene Sergeev <eugeney.sergeev at gmail.com>
 * Please check copyright notice at the bottom of pllua.h
 */

#ifndef PLLUA_HSTORE_H
#define PLLUA_HSTORE_H


#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <postgres.h>
#include <hstore.h>

void register_hstore(lua_State * L);
int register_hstore_tn(lua_State * L, const char * pg_schema_name);

void register_hstore_func(lua_State * L);

Datum getHStoreDatum(lua_State *L, int index);
void setHstoreFromDatum(lua_State *L, Datum datum);


#endif // PLLUA_HSTORE_H
