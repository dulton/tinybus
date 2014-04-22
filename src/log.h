#ifndef _LOG_MOD_H_
#define _LOG_MOD_H_

#include "tinybus.h"

typedef struct _log_module
{
	slot_t 	*slot;
} log_module_t;

log_mod_t *log_module_new();
void log_module_free(log_mod_t *mod);

#endif
