#include "pllua_hstore.h"


#include "plluacommon.h"
#include "pllua.h"

static struct {
    int oid;
    int16 len;
    char type;
    char align;
    bool byval;
    Oid elem;
    FmgrInfo input;
    FmgrInfo output;
    TupleDesc tupdesc;
} hs_type;

static const char hstore_type_name[] = "hstore";

typedef struct Lua_Hstore{
    HStore *hstore;
    Datum datum;
    int issync;
    int havetodel; // always copy, always del.
}Lua_Hstore;

static void set_pair(lua_State *L,
                     Pairs *pairs,
                     HEntry	   *entries,
                     char	   *base,
                     int		i){
    char * kvalue;
    int klen;

    MTOLUA(L);
    kvalue = NULL;
    klen = HS_KEYLEN(entries, i);
    if (klen){
        kvalue = (char*)palloc(klen+1);
        memset(kvalue,0,klen+1);
        memcpy(kvalue,HS_KEY(entries, base, i),klen);
    }
    pairs->key = kvalue;
    pairs->keylen = klen;
    pairs->needfree = true;
    if (HS_VALISNULL(entries, i))
    {
        pairs->val = NULL;
        pairs->vallen = 0;
        pairs->isnull = true;
    } else
    {
        char * vvalue = NULL;
        int vlen = HS_VALLEN(entries, i);

        if (vlen){
            vvalue = (char*)palloc(vlen+1);
            memset(vvalue,0,vlen+1);
            memcpy(vvalue,HS_VAL(entries, base, i),vlen);
        }

        pairs->val = vvalue;
        pairs->vallen = vlen;
        pairs->isnull = false;
    }
    MTOPG;
}

static void set_pair_from_lv(lua_State *L,
                             Pairs *pairs,
                             const char *key,
                             char *mov_value)
{
    char * kvalue;
    int klen;

    MTOLUA(L);
    kvalue = NULL;
    klen = strlen(key);
    if (klen){
        kvalue = (char*)palloc(klen+1);
        memset(kvalue,0,klen+1);
        memcpy(kvalue, key, klen);
    }
    pairs->key = kvalue;
    pairs->keylen = klen;
    pairs->needfree = true;
    if (mov_value == NULL)
    {
        pairs->val = NULL;
        pairs->vallen = 0;
        pairs->isnull = true;
    } else  {

        int vlen = strlen(mov_value);
        pairs->val = mov_value;
        pairs->vallen = vlen;
        pairs->isnull = false;
    }
    MTOPG;
}

static void pg_datumFree(Datum value, bool typByVal, int typLen)
{
    if (!typByVal)
    {
        void     *s = DatumGetPointer(value);
        pfree(s);
    }
}

static void apply_changes(lua_State *L, Lua_Hstore *strg){
    HStore	   *hsout;
    int32		buflen;
    HStore	   *in;
    int			i;
    int			count;
    int         pcount;
    char	   *base;
    HEntry	   *entries;
    Pairs	   *pairs;

    if (strg->issync == 1) return;

    in = strg->hstore;
    count = HS_COUNT(in);
    base = STRPTR(in);
    entries = ARRPTR(in);

    lua_pushlightuserdata(L, strg);
    lua_gettable(L, LUA_REGISTRYINDEX);

    for (i=0; i < count; i++)
    {
        char *key = HS_KEY(entries, base, i);
        int klen = HS_KEYLEN(entries, i);
        lua_pushlstring(L, key, klen);
        lua_rawget(L, -2);
        if (lua_isnil(L,-1)){
            lua_pop(L, 1);
            lua_pushlstring(L, key, klen);
            lua_pushinteger(L, i);
            lua_rawset(L, -3);
        }else {
            lua_pop(L, 1);
        }
    }

    pcount = 0;
    lua_pushnil(L);  /* first key */

    while (lua_next(L, -2) != 0) {
        ++pcount;
        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
    }

    MTOLUA(L);
    pairs = palloc(pcount * sizeof(*pairs));
    MTOPG;
    i = 0;
    //-------------------------------
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        const char *key = lua_tostring(L, -2);
        int ltype = lua_type(L, -1);

        if (ltype == LUA_TNUMBER){
            int idx = lua_tointeger(L, -1);
            set_pair(L, &pairs[i], entries, base, idx);
        } else {
            char *mov_value = (char *)lua_touserdata (L, -1);
            set_pair_from_lv(L, &pairs[i], key, mov_value);
        }
        lua_pop(L, 1);
        ++i;
    }
    //-------------------------------
    lua_pop(L, 1);

    MTOLUA(L);
    pcount = hstoreUniquePairs(pairs, pcount, &buflen);
    hsout = hstorePairs(pairs, pcount, buflen);
    MTOPG;


    if (strg->havetodel){
        pg_datumFree(strg->datum, hs_type.byval, hs_type.len);
    }
    strg->hstore = hsout;
    strg->datum = PointerGetDatum(hsout);
    strg->issync = 1;
    strg->havetodel = 1;

    lua_pushlightuserdata(L, strg);
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);
}


static int hstore_index(lua_State *L) {

    HStore     *hs;
    char	   *base;
    HEntry	   *entries;
    const char *key;
    char       *k_str;
    int         idx;
    Lua_Hstore *strg;

    BEGINLUA;
    strg = lua_touserdata(L, 1);

    hs = strg->hstore;
    base = STRPTR(hs);
    entries = ARRPTR(hs);

    key = luaL_checkstring(L, 2);

    lua_pushlightuserdata(L, strg);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lua_pushstring(L, key);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {
        char *data_ptr = lua_touserdata (L, -1);
        lua_pop(L,2);

        if (data_ptr){
            lua_pushstring(L, data_ptr);
        }else {
            lua_pushnil(L);
        }

        ENDLUAV(1);
        return 1;
    }

    lua_pop(L,2);

    k_str = strdup(key);

    idx = hstoreFindKey(hs, NULL, k_str, strlen(k_str));

    free(k_str);
    if (idx<0|| HS_VALISNULL(entries, idx)){
        lua_pushnil(L);
        ENDLUAV(1);
        return 1;
    }

    lua_pushlstring(L, HS_VAL(entries, base, idx),HS_VALLEN(entries, idx));
    ENDLUAV(1);
    return 1;
}


static int hstore_newindex(lua_State *L) {
    //hs["foo"] = "bar"
    char * value;
    const char *buf;
    Lua_Hstore *strg = lua_touserdata(L, 1);//hs
    const char *key = luaL_checkstring(L, 2);//"foo"
    if (lua_isnil(L,3)){
        buf = NULL;
    } else {
        buf = luaL_checkstring(L, 3);//"bar"
    }

    strg->issync = 0;
    lua_pushlightuserdata(L, strg);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lua_pushstring(L, key);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {
        //free old value
        void *data_ptr = lua_touserdata (L, -1);
        if (data_ptr){
            pfree(data_ptr);
        }
    }
    lua_pop(L, 1); // hs/"foo"/"bar"/table/(nil or val) -> pop(nil or val)
    MTOLUA(L);
    value = NULL;
    if (buf){
        value = (char*)palloc(strlen(buf)+1);
        memset(value,0,strlen(buf)+1);
        memcpy(value,buf,strlen(buf));
    }
    lua_pushstring(L, key);// hs/"foo"/"bar"/table/"foo"
    lua_pushlightuserdata(L, value);// hs/"foo"/"bar"/table/"foo"/*bar
    lua_settable(L, -3);// hs/key/value/table <-(["foo"] = *bar)
    MTOPG;
    lua_pop(L, 4);
    return 0;
}


static void __newmetatable (lua_State *L, const char *tname) {
    lua_newtable(L);
    lua_pushlightuserdata(L, (void *) tname);
    lua_pushvalue(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);
}


int hstore_oid = 0;
int registered = 0;

static int get_hstore_oid(const char* pg_schema_name)
{

    if (hstore_oid > 0) return hstore_oid;
    if (pg_schema_name == NULL)
        hstore_oid = pg_to_regtype("hstore");
    else{
        char buf[strlen(pg_schema_name)+strlen(".hstore")+1];
        sprintf(buf,"%s.hstore", pg_schema_name);
        hstore_oid = pg_to_regtype(&buf[0]);
    }
    return hstore_oid;
}

static int tostring(lua_State *L) {
    Lua_Hstore *strg = lua_touserdata(L, 1);
    apply_changes(L, strg);
    if (strg->issync){
        lua_pushstring(L, OutputFunctionCall(&hs_type.output, PointerGetDatum(strg->hstore)));
    }else{
        return luaL_error(L, "hstore apply changes failed");
    }
    return 1;
}

static int gc_hstore(lua_State *L) {
    Lua_Hstore *strg = lua_touserdata(L, 1);

    lua_pushlightuserdata(L, strg);
    lua_gettable(L, LUA_REGISTRYINDEX);

    lua_pushnil(L);  /* first key */
    while (lua_next(L, -2) != 0) {
        char *pvalue = (char *)lua_touserdata (L, -1);
        if(pvalue){
            pfree(pvalue);
        }
        lua_pop(L, 1);
    }

    lua_pop(L, 1);

    if (strg->havetodel){
        pg_datumFree(strg->datum, hs_type.byval, hs_type.len);
    }

    lua_pushlightuserdata(L, strg);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);

    return 0;
}

static luaL_Reg regs[] =
{
    {"__index", hstore_index},
    {"__newindex", hstore_newindex},
    {"__tostring", tostring},
    {"__gc", gc_hstore},

    { NULL, NULL }
};

static Oid delete_hstore_text_oid = InvalidOid;

static void init_delete_hstore_text(lua_State *L, const char* hstore_schema){

    char funcname[2*strlen(hstore_schema)+strlen(".delete(.hstore,text)")+1];

    if (OidIsValid(delete_hstore_text_oid))return;

    if (hstore_schema == NULL){
        sprintf(funcname,"%s", "delete(hstore,text)");
    }else{
        sprintf(funcname, "%s.delete(%s.hstore,text)", hstore_schema, hstore_schema);
    }

    delete_hstore_text_oid = DatumGetObjectId(DirectFunctionCall1(regprocedurein, CStringGetDatum(funcname)));
    if (!OidIsValid(delete_hstore_text_oid)){
        luaL_error(L,"failed to register delete(hstore,text)");
    }
}

static Datum call_delete_hstore_text(Datum hstore_0, Datum text_1){
    return OidFunctionCall2(delete_hstore_text_oid, hstore_0, text_1);
}



#define luaP_getfield(L, s) \
    lua_pushlightuserdata((L), (void *)(s)); \
    lua_rawget((L), LUA_REGISTRYINDEX)

static int hs_delete(lua_State *L) {

    if (lua_gettop(L)!= 2){
        return luaL_error(L, "wrong args length in hstore.delete(hstore, text)");
    }
    if (hstore_oid >0){
        bool isnull;
        Datum a;
        Datum b;

        if ((lua_type(L, 2) != LUA_TSTRING)||
                (lua_type(L, 1) != LUA_TUSERDATA)
                ){
            return luaL_error(L, "wrong args types for hstore.delete(hstore, text)");
        }
        lua_getmetatable (L, 1);
        luaP_getfield(L, hstore_type_name);
        if (lua_equal (L, -1, -2)!= 1){
            return luaL_error(L, "hstore.delete(hstore, text) first arg is not hstore");
        }

        a = luaP_todatum(L, hstore_oid, 0, &isnull, 1);
        b = luaP_todatum(L, TEXTOID, 0, &isnull, 2);
        luaP_pushdatum(L, call_delete_hstore_text(a,b), hstore_oid);
        return 1;


    }
    return luaL_error(L, "hstore is not registered //hstore.delete");
}

static luaL_Reg func_regs[] =
{
    { "delete", hs_delete },
    { NULL, NULL }
};


#include "pllua_xact_cleanup.h"



void register_hstore_func(lua_State *L)
{
    luaL_newmetatable(L, hstore_type_name);
    luaL_setfuncs(L, func_regs, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    lua_setglobal(L, hstore_type_name);
}


static int reg_hstore(lua_State *L, const char* pg_schema_name){

    MemoryContext mcxt;
    HeapTuple	type;
    Form_pg_type typeinfo;
    int oid;

    if (registered == 1) return hstore_oid;

    oid = get_hstore_oid(pg_schema_name);

    if (oid <= 0) return oid;


    init_delete_hstore_text(L, pg_schema_name);

    __newmetatable(L, hstore_type_name);
    luaP_register(L, regs);
    lua_pop(L, 1);

    type = SearchSysCache(TYPEOID, ObjectIdGetDatum(oid), 0, 0, 0);
    if (!HeapTupleIsValid(type))
        elog(ERROR, "[pllua]: cache lookup failed for type %u", oid);
    typeinfo = (Form_pg_type) GETSTRUCT(type);
    hs_type.len = typeinfo->typlen;
    hs_type.type = typeinfo->typtype;
    hs_type.align = typeinfo->typalign;
    hs_type.byval = typeinfo->typbyval;
    hs_type.elem = typeinfo->typelem;
    mcxt = get_common_ctx();
    fmgr_info_cxt(typeinfo->typinput, &hs_type.input, mcxt);
    fmgr_info_cxt(typeinfo->typoutput, &hs_type.output, mcxt);

    ReleaseSysCache(type);
    registered = 1;
    return oid;
}

void register_hstore(lua_State *L)
{
    reg_hstore(L, NULL);
}
int register_hstore_tn(lua_State * L, const char * pg_schema_name)
{
    return reg_hstore(L, pg_schema_name);
}



void setHstoreFromDatum(lua_State *L, Datum datum)
{
    Lua_Hstore * strg;

    BEGINLUA;

    strg = (Lua_Hstore *)lua_newuserdata(L, sizeof(Lua_Hstore));
    MTOLUA(L);
    datum = datumCopy(datum, hs_type.byval, hs_type.len);
    MTOPG;

    strg->hstore = DatumGetHStoreP(datum);
    strg->datum = datum;
    strg->issync = 1;

    strg->havetodel = 1;

    lua_pushlightuserdata(L, strg);
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);

    luaP_getfield(L, hstore_type_name);
    lua_setmetatable(L, -2);
    ENDLUAV(1);
}

Datum getHStoreDatum(lua_State *L, int index){
    Lua_Hstore *strg = lua_touserdata(L, index);
    apply_changes(L, strg);
    strg->havetodel = 1;
    return strg->datum;
}


