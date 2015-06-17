#include "plluacommon.h"

MemoryContext luaP_getmemctxt(lua_State *L) {
    MemoryContext mcxt;
    lua_pushlightuserdata(L, (void *) L);
    lua_rawget(L, LUA_REGISTRYINDEX);
    mcxt = (MemoryContext) lua_touserdata(L, -1);
    lua_pop(L, 1);
    return mcxt;
}
