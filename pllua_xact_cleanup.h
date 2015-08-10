#ifndef PLLUA_XACT_CLEANUP_H
#define PLLUA_XACT_CLEANUP_H

#include "plluacommon.h"

typedef void (*RSDtorCallback) (void *data);

void pllua_init_common_ctx(void);
void pllua_delete_common_ctx(void);
void pllua_xact_cb(XactEvent event, void *arg);

void *register_resource(void *d, RSDtorCallback dtor);
void *unregister_resource(void* d);


#endif // PLLUA_XACT_CLEANUP_H
