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

#define info(msg) ereport(INFO, (errmsg("%s", msg)))

#define MTOLUA(state) MemoryContext ___mcxt = luaP_getmemctxt(state); \
MemoryContext ___m = MemoryContextSwitchTo(___mcxt);

#define MTOPG MemoryContextSwitchTo(___m);

#endif // PLLUACOMMON_H
