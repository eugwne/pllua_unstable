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
    TupleDesc tupleDesc;
} datatuptable_t;

static tsk_object_t* datatuptable_ctor(tsk_object_t * self, va_list * app)
{
    TupleDesc desc;
    datatuptable_t *d = (datatuptable_t *)self;
    if(d){
        d->L = va_arg(*app,lua_State *);
        desc = va_arg(*app, TupleDesc);
         MTOLUA(d->L);
        d->tupleDesc = CreateTupleDescCopy(desc);
        MTOPG;

    }
    return self;
}

static tsk_object_t * datatuptable_dtor(tsk_object_t * self)
{
    datatuptable_t *d = (datatuptable_t *)self;
      if(d){
        MTOLUA(d->L);
        FreeTupleDesc(d->tupleDesc);
        MTOPG;
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
