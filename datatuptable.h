#ifndef DATATUPTABLE_H
#define DATATUPTABLE_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "tinySAK/tsk.h"

#include "plluacommon.h"
#include <c.h>
typedef struct datatuptable_s
{
    TSK_DECLARE_OBJECT;
    lua_State *L;
    int colCount;
    NameData *cols;
    //TupleDesc tupleDesc; caused strange behavior...
} datatuptable_t;

static tsk_object_t* datatuptable_ctor(tsk_object_t * self, va_list * app)
{
    int i;
    TupleDesc desc;
    datatuptable_t *d = (datatuptable_t *)self;
    if(d){
        MemoryContext mcxt;
        MemoryContext m;
        d->L = va_arg(*app,lua_State *);

        desc = va_arg(*app, TupleDesc);
        d->colCount = desc->natts;
        mcxt = luaP_getmemctxt(d->L);
        m = MemoryContextSwitchTo(mcxt);

        d->cols = palloc(sizeof(NameData)*d->colCount);
        for (i = 0; i < desc->natts; i++) {
            memcpy(&d->cols[i].data, desc->attrs[i]->attname.data,NAMEDATALEN);
        }

        MemoryContextSwitchTo(m);

    }
    return self;
}
#define info(msg) ereport(INFO, (errmsg("%s", msg)))
static tsk_object_t * datatuptable_dtor(tsk_object_t * self)
{
    datatuptable_t *d = (datatuptable_t *)self;

    info("gc tuple");


    if(d){
        MemoryContext mcxt;
        MemoryContext m;
        mcxt = luaP_getmemctxt(d->L);
        m = MemoryContextSwitchTo(mcxt);
        pfree(d->cols);
        MemoryContextSwitchTo(m);
    }
    return self;
}


static const tsk_object_def_t datatuptable_def_t =
{
    sizeof(datatuptable_t),
    datatuptable_ctor,
    datatuptable_dtor,
    NULL
};

#endif // DATATUPTABLE_H
