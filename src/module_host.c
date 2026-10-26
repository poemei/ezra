#include <stdatomic.h>
#include <string.h>
#include "module_host.h"
#define EZRA_HOST_SERVICE_MAX 32U
#define EZRA_HOST_COMMAND_MAX 32U
typedef struct {
 char name[STNLABZ_MODULE_SERVICE_NAME_MAX];
 stnlabz_module_service_handler_fn handler;
 void *context;
 unsigned int in_flight;
 int withdrawing;
} service_t;
typedef struct {
 char name[STNLABZ_MODULE_COMMAND_NAME_MAX];
 stnlabz_module_command_handler_fn handler;
 void *context;
} command_t;
static command_t commands[EZRA_HOST_COMMAND_MAX];
static atomic_flag command_lock = ATOMIC_FLAG_INIT;
static service_t services[EZRA_HOST_SERVICE_MAX];
static atomic_flag service_lock = ATOMIC_FLAG_INIT;
static void lock(void) { while (atomic_flag_test_and_set_explicit(&service_lock, memory_order_acquire)) {} }
static void unlock(void) { atomic_flag_clear_explicit(&service_lock, memory_order_release); }
static size_t len(const char *s,size_t max) { size_t i=0; if (!s) return max; while (i<max && s[i]) i++; return i; }
static int register_service(const char *name, stnlabz_module_service_handler_fn handler, void *ctx) {
 size_t i,free_slot=EZRA_HOST_SERVICE_MAX,n=len(name,STNLABZ_MODULE_SERVICE_NAME_MAX);
 if (!handler || !n || n>=STNLABZ_MODULE_SERVICE_NAME_MAX) return 0;
 lock();
 for(i=0;i<EZRA_HOST_SERVICE_MAX;i++) {
   if (services[i].handler && !strcmp(services[i].name,name)) {unlock();return 0;}
   if (!services[i].handler && free_slot==EZRA_HOST_SERVICE_MAX) free_slot=i;
 }
 if (free_slot==EZRA_HOST_SERVICE_MAX) {unlock();return 0;}
 memcpy(services[free_slot].name,name,n+1);
 services[free_slot].handler=handler;
 services[free_slot].context=ctx;
 unlock();return 1;
}
static int unregister_service(const char *name,void *ctx) {
 size_t i; service_t *entry=NULL;
 if (!name) return 0;
 lock();
 for(i=0;i<EZRA_HOST_SERVICE_MAX;i++)
 if(services[i].handler && !services[i].withdrawing &&
    services[i].context==ctx && !strcmp(services[i].name,name)) {
    services[i].withdrawing=1;entry=&services[i];break;
 }
 unlock();if (!entry) return 0;
 for (;;) { lock();if(!entry->in_flight) {memset(entry,0,sizeof *entry);unlock();return 1;}unlock(); }
}
static stnlabz_module_result_t invoke_service(const char *name,const void *request,size_t request_size,
 void *response,size_t response_size,size_t *response_used) {
 size_t i;service_t *entry=NULL;
 stnlabz_module_service_handler_fn handler=NULL;void *ctx=NULL;
 stnlabz_module_result_t result;
 if(response_used) *response_used=0;
 if(!name || (request_size && !request) || (response_size && !response)) return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
 lock();
 for(i=0;i<EZRA_HOST_SERVICE_MAX;i++)
 if(services[i].handler && !services[i].withdrawing && !strcmp(services[i].name,name)){
 entry=&services[i];entry->in_flight++;handler=entry->handler;ctx=entry->context;break;}
 unlock();
 if(!handler) return STNLABZ_MODULE_ERR_NOT_FOUND;
 result=handler(request,request_size,response,response_size,response_used,ctx);
 lock();entry->in_flight--;unlock();return result;
}
static int register_command(const char *name, stnlabz_module_command_handler_fn handler,void *ctx) {
 size_t i,slot=EZRA_HOST_COMMAND_MAX,n=len(name,STNLABZ_MODULE_COMMAND_NAME_MAX);
 if(!handler || !n || n>=STNLABZ_MODULE_COMMAND_NAME_MAX) return 0;
 while(atomic_flag_test_and_set_explicit(&command_lock,memory_order_acquire)){}
 for(i=0;i<EZRA_HOST_COMMAND_MAX;i++) {
  if(commands[i].handler && !strcmp(commands[i].name,name)) break;
  if(!commands[i].handler && slot==EZRA_HOST_COMMAND_MAX) slot=i;
 }
 if(i==EZRA_HOST_COMMAND_MAX && slot!=EZRA_HOST_COMMAND_MAX) {
  memcpy(commands[slot].name,name,n+1);commands[slot].handler=handler;commands[slot].context=ctx;
 }
 atomic_flag_clear_explicit(&command_lock,memory_order_release);
 return i==EZRA_HOST_COMMAND_MAX && slot!=EZRA_HOST_COMMAND_MAX;
}
static int unregister_command(const char *name,void *ctx) {
 size_t i;int result=0;
 if(!name) return 0;
 while(atomic_flag_test_and_set_explicit(&command_lock,memory_order_acquire)){}
 for(i=0;i<EZRA_HOST_COMMAND_MAX;i++)
  if(commands[i].handler && commands[i].context==ctx && !strcmp(commands[i].name,name)) {
    memset(&commands[i],0,sizeof commands[i]);result=1;break;
  }
 atomic_flag_clear_explicit(&command_lock,memory_order_release);
 return result;
}
static const stnlabz_module_host_t host={
 .register_command=register_command,.unregister_command=unregister_command,
 .register_service=register_service,.unregister_service=unregister_service,.invoke_service=invoke_service
};
const stnlabz_module_host_t *ezra_module_host(void){return &host;}
