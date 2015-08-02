#ifndef RTUPDESCSTK_H
#define RTUPDESCSTK_H

#include "plluacommon.h"

#include <stdlib.h>
struct stackType;

typedef struct RTDnode {
    void *data;
    struct RTDnode *next;
    struct RTDnode *prev;
    struct stackType *tail;
} RTDNode, *RTDNodePtr;

typedef struct stackType {
    int ref_count;
    lua_State *L;
    RTDNodePtr top;
} RTupDescStackType, *RTupDescStack;

RTupDescStack rtds_set_current(void *s);
RTupDescStack rtds_get_current(void);

int rtds_get_length(RTupDescStack S);


RTupDescStack rtds_initStack(lua_State *L);

int rtds_isempty(RTupDescStack S);

RTDNodePtr rtds_push(RTupDescStack S, void *d);

RTDNodePtr rtds_push_current(void *d);

void *rtds_pop(RTupDescStack S);

void rtds_clean(RTupDescStack S);

void rtds_remove_node(RTDNodePtr np);

RTupDescStack rtds_unref(RTupDescStack S);

void rtds_inuse(RTupDescStack S);

void rtds_notinuse(RTupDescStack S);

RTupDescStack rtds_free_if_not_used(RTupDescStack S);




#endif // RTUPDESCSTK_H
