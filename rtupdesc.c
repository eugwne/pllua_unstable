#include "rtupdesc.h"


RTupDesc *rtupdesc_ctor(lua_State *state, TupleDesc tupdesc, int free_tupdesc)
{
    void* p;
    RTupDesc* rtupdesc = 0;

    if (free_tupdesc == 1){
        MTOLUA(state);
        p = palloc(sizeof(RTupDesc));
        MTOPG;
    }else {
        p = palloc(sizeof(RTupDesc));
    }
    if (p){
        rtupdesc = (RTupDesc*)p;
        rtupdesc->ref_count = 1;
        rtupdesc->L = state;
        if (free_tupdesc == 1){
            MTOLUA(state);
            rtupdesc->tupdesc = CreateTupleDescCopy(tupdesc);
            MTOPG;
        }else{
            rtupdesc->tupdesc = tupdesc;
        }
        rtupdesc->free_tupdesc = free_tupdesc;
    }


    return rtupdesc;
}


RTupDesc *rtupdesc_ref(RTupDesc *rtupdesc)
{
    if(rtupdesc)
        rtupdesc->ref_count += 1;
    return rtupdesc;
}


RTupDesc *rtupdesc_unref(RTupDesc *rtupdesc)
{
    if(rtupdesc){
        rtupdesc->ref_count -= 1;
        if (rtupdesc->ref_count == 0){
            rtupdesc_dtor(rtupdesc);
        }
    }
    return rtupdesc;
}


void rtupdesc_dtor(RTupDesc *rtupdesc)
{
    if(rtupdesc){
        if(rtupdesc->free_tupdesc == 1){
            MTOLUA(rtupdesc->L);
            FreeTupleDesc(rtupdesc->tupdesc);
            pfree(rtupdesc);
            MTOPG;
        }else{
            pfree(rtupdesc);
        }

        rtupdesc = 0;

    }
}
