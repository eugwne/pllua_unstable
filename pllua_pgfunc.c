#include "pllua_pgfunc.h"

#include "plluacommon.h"
#include "pllua.h"
#include "pllua_xact_cleanup.h"

#include <catalog/pg_language.h>

static const char pg_func_type_name[] = "pg_func";

typedef struct{
    Oid         funcid;
    int			numargs;
    Oid		   *argtypes;
    char	  **argnames;
    char	   *argmodes;
    lua_CFunction  callfunc;
    Oid         prorettype;
} PgFuncInfo, Lua_pgfunc;

#define freeandnil(p)    do{ if (p){\
    pfree(p);\
    p = NULL;\
    }}while(0)

static void clean_pgfuncinfo(Lua_pgfunc *data){
    freeandnil (data->argtypes);
    freeandnil (data->argnames);
    freeandnil (data->argmodes);
}


static int call_0(lua_State *L){
    Lua_pgfunc *fi = (Lua_pgfunc *) lua_touserdata(L, lua_upvalueindex(1));
    luaP_pushdatum(L, OidFunctionCall0(fi->funcid), fi->prorettype);
    return 1;
}

#define g_datum(v) luaP_todatum(L, fi->argtypes[v], 0, &isnull, v+1)
static int call_1(lua_State *L)
{
    Lua_pgfunc *fi = (Lua_pgfunc *) lua_touserdata(L, lua_upvalueindex(1));
    bool isnull;

    luaP_pushdatum(L, OidFunctionCall1(fi->funcid,
                                       g_datum(0)), fi->prorettype);
    return 1;
}
static int call_2(lua_State *L)
{
    Lua_pgfunc *fi = (Lua_pgfunc *) lua_touserdata(L, lua_upvalueindex(1));
    bool isnull;

    luaP_pushdatum(L, OidFunctionCall2(fi->funcid,
                                       g_datum(0),
                                       g_datum(1)), fi->prorettype);
    return 1;
}
static int call_3(lua_State *L)
{
    Lua_pgfunc *fi = (Lua_pgfunc *) lua_touserdata(L, lua_upvalueindex(1));
    bool isnull;

    luaP_pushdatum(L, OidFunctionCall3(fi->funcid,
                                       g_datum(0),
                                       g_datum(1),
                                       g_datum(2)), fi->prorettype);
    return 1;
}
static int call_4(lua_State *L)
{
    Lua_pgfunc *fi = (Lua_pgfunc *) lua_touserdata(L, lua_upvalueindex(1));
    bool isnull;

    luaP_pushdatum(L, OidFunctionCall4(fi->funcid,
                                       g_datum(0),
                                       g_datum(1),
                                       g_datum(2),
                                       g_datum(3)), fi->prorettype);
    return 1;
}
#undef g_datum



static lua_CFunction pg_callable_funcs[] =
{
    call_0,
    call_1,
    call_2,
    call_3,
    call_4,
    NULL
};

#define luaP_getfield(L, s) \
    lua_pushlightuserdata((L), (void *)(s)); \
    lua_rawget((L), LUA_REGISTRYINDEX)

int get_pgfunc(lua_State *L)
{
    Lua_pgfunc *lf;
    MemoryContext mcxt;
    MemoryContext m;
    const char* reg_name;
    HeapTuple	proctup;

    Form_pg_proc proc;
    const char* reg_error = NULL;
    int i;
    Oid funcid = 0;


    if (lua_gettop(L) != 1){
        return luaL_error(L, "pgfunc(text): wrong arguments");
    }
    if(lua_type(L, 1) == LUA_TSTRING){
        reg_name = luaL_checkstring(L, 1);

        PG_TRY();
        {
            funcid = DatumGetObjectId(DirectFunctionCall1(regprocedurein, CStringGetDatum(reg_name)));
        }
        PG_CATCH();{}
        PG_END_TRY();
    }else if (lua_type(L, 1) == LUA_TNUMBER){
        funcid = luaL_checkinteger(L, 1);
    }

    if (!OidIsValid(funcid)){
        return luaL_error(L,"failed to register %s", reg_name);
    }

    proctup = SearchSysCache1(PROCOID, ObjectIdGetDatum(funcid));
    if (!HeapTupleIsValid(proctup)){
        return luaL_error(L,"cache lookup failed for function %d", funcid);
    }

    proc = (Form_pg_proc) GETSTRUCT(proctup);

    if ((proc->prolang != INTERNALlanguageId)&&(proc->prolang !=SQLlanguageId)){
        ReleaseSysCache(proctup);
        return luaL_error(L, "supported only SQL/internal functions");
    }


    lf = (Lua_pgfunc *)lua_newuserdata(L, sizeof(Lua_pgfunc));

    //make it g/collected
    luaP_getfield(L, pg_func_type_name);
    lua_setmetatable(L, -2);


    lf->prorettype = proc->prorettype;
    lf->funcid = funcid;


    mcxt = get_common_ctx();
    m  = MemoryContextSwitchTo(mcxt);

    lf->numargs = get_func_arg_info(proctup,
                                    &lf->argtypes, &lf->argnames, &lf->argmodes);

    m  = MemoryContextSwitchTo(m);

    if (lf->numargs >4){
        reg_error = "not supported function with more than 4 arguments";
    }else {
        for (i = 0; i < lf->numargs; i++){
            char		argmode = lf->argmodes ? lf->argmodes[i] : PROARGMODE_IN;
            if (argmode != PROARGMODE_IN){
                reg_error = "only input parameters supported";
                break;
            }
        }
    }
    if (reg_error){
        ReleaseSysCache(proctup);
        clean_pgfuncinfo(lf);
        return luaL_error(L, "pgfunc error: %s",reg_error);
    }

    lua_pushcclosure(L, pg_callable_funcs[lf->numargs], 1);

    ReleaseSysCache(proctup);

    return 1;
}

static void __newmetatable (lua_State *L, const char *tname)
{
    lua_newtable(L);
    lua_pushlightuserdata(L, (void *) tname);
    lua_pushvalue(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

static int gc_pg_func(lua_State *L)
{
    Lua_pgfunc *lf = lua_touserdata(L, 1);
    clean_pgfuncinfo(lf);
    return 0;
}

static luaL_Reg regs[] =
{
    {"__gc", gc_pg_func},
    { NULL, NULL }
};

void register_funcinfo_mt(lua_State *L)
{
    __newmetatable(L, pg_func_type_name);
    luaP_register(L, regs);
    lua_pop(L, 1);
}
