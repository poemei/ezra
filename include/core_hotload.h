#ifndef EZRA_CORE_HOTLOAD_H
#define EZRA_CORE_HOTLOAD_H
#include "core_policy.h"
#include "stnlabz_abi/module_loader.h"
#include "stnlabz_abi/module_registry.h"
typedef struct ezra_hotload ezra_hotload_t;
ezra_hotload_t *ezra_hotload_create(const char *directory,
 stnlabz_module_registry_t *registry,stnlabz_module_loader_t *loader,
 const stnlabz_module_host_t *host,const ezra_policy_request_t *policy);
void ezra_hotload_poll(ezra_hotload_t *watch);
void ezra_hotload_destroy(ezra_hotload_t *watch);
#endif
