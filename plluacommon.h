#ifndef PLLUACOMMON_H
#define PLLUACOMMON_H

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <funcapi.h>
#include <access/heapam.h>
#if PG_VERSION_NUM >= 90300
#include <access/htup_details.h>
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* get MemoryContext for state L */
MemoryContext luaP_getmemctxt (lua_State *L);

#endif // PLLUACOMMON_H
