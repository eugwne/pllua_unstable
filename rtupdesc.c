#include "rtupdesc.h"


RTupDesc *rtupdesc_ctor(lua_State *state, TupleDesc tupdesc)
{
    void* p;
    RTupDesc* rtupdesc = 0;


    MTOLUA(state);
    p = palloc(sizeof(RTupDesc));
    if (p){
        rtupdesc = (RTupDesc*)p;
        rtupdesc->ref_count = 1;
        rtupdesc->L = state;
        rtupdesc->tupdesc = CreateTupleDescCopy(tupdesc);
    }
    MTOPG;


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

        MTOLUA(rtupdesc->L);
        FreeTupleDesc(rtupdesc->tupdesc);
        pfree(rtupdesc);
        MTOPG;

        rtupdesc = 0;

    }
}
