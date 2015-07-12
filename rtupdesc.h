#ifndef RTUPDESC_H
#define RTUPDESC_H

#include "plluacommon.h"

typedef struct _RTupDesc{
    int ref_count;
    lua_State *L;
    TupleDesc tupdesc;
    int free_tupdesc;
} RTupDesc;

RTupDesc* rtupdesc_ctor(lua_State * state, TupleDesc tupdesc, int free_tupdesc);

RTupDesc* rtupdesc_ref(RTupDesc* rtupdesc);

RTupDesc* rtupdesc_unref(RTupDesc* rtupdesc);

void rtupdesc_dtor(RTupDesc* rtupdesc);


#endif // RTUPDESC_H
