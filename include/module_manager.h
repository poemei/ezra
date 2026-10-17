#ifndef EZRA_MODULE_MANAGER_H
#define EZRA_MODULE_MANAGER_H
#include <stddef.h>
#define EZRA_MODULE_MAX 32
#define EZRA_MODULE_ID_MAX 64
#define EZRA_MODULE_ABI_MAJOR 1U
#define EZRA_MODULE_ABI_MINOR 0U
#define EZRA_MODULE_MIN_TESTS 1U

typedef enum {
    EZRA_MODULE_REGISTERED = 0,
    EZRA_MODULE_QUALIFIED = 1,
    EZRA_MODULE_ACTIVE = 2,
    EZRA_MODULE_STOPPED = 3
} ezra_module_state_t;
typedef enum {
    EZRA_MODULE_OK = 0,
    EZRA_MODULE_INVALID = 1,
    EZRA_MODULE_DUPLICATE = 2,
    EZRA_MODULE_FULL = 3,
    EZRA_MODULE_NOT_FOUND = 4,
    EZRA_MODULE_UNQUALIFIED = 5,
    EZRA_MODULE_CALLBACK_FAILED = 6
} ezra_module_result_t;
typedef struct {
    char id[EZRA_MODULE_ID_MAX];
    unsigned int abi_major, abi_minor;
    unsigned int version_major, version_minor, version_patch;
} ezra_module_descriptor_t;
typedef struct {
    unsigned int tests_executed, tests_passed, tests_failed;
    int negative_test_executed, negative_test_passed;
} ezra_module_qualification_t;
typedef struct {
    int (*start)(void *context);
    void (*stop)(void *context);
    void *context;
} ezra_module_callbacks_t;
typedef struct {
    ezra_module_descriptor_t descriptor;
    ezra_module_qualification_t qualification;
    ezra_module_callbacks_t callbacks;
    ezra_module_state_t state;
    int activation_authorized;
} ezra_module_record_t;
typedef struct {
    ezra_module_record_t modules[EZRA_MODULE_MAX];
    size_t count;
} ezra_module_manager_t;

void ezra_module_manager_init(ezra_module_manager_t *manager);
ezra_module_result_t ezra_module_register(ezra_module_manager_t *manager,
    const ezra_module_descriptor_t *descriptor, const ezra_module_callbacks_t *callbacks);
ezra_module_result_t ezra_module_qualify(ezra_module_manager_t *manager,
    const char *id, const ezra_module_qualification_t *evidence);
ezra_module_result_t ezra_module_authorize(ezra_module_manager_t *manager,
    const char *id, int operator_authorized);
ezra_module_result_t ezra_module_activate(ezra_module_manager_t *manager, const char *id);
ezra_module_result_t ezra_module_stop(ezra_module_manager_t *manager, const char *id);
void ezra_module_manager_shutdown(ezra_module_manager_t *manager);
const ezra_module_record_t *ezra_module_find(const ezra_module_manager_t *manager, const char *id);
#endif
