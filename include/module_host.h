#ifndef EZRA_MODULE_HOST_H
#define EZRA_MODULE_HOST_H

#include "stnlabz_abi/module.h"

const stnlabz_module_host_t *ezra_module_host(void);
void ezra_module_host_begin_registration(const char *module_id);
void ezra_module_host_end_registration(void);

#endif
