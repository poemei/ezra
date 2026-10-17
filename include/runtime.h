#ifndef EZRA_RUNTIME_H
#define EZRA_RUNTIME_H

typedef enum {
    EZRA_OPERATIONAL_NORMAL = 0,
    EZRA_OPERATIONAL_SAFE_MODE = 1,
    EZRA_OPERATIONAL_COMPANY_PRESERVATION = 2
} ezra_operational_state_t;

typedef enum {
    EZRA_FAULT_AUTHORITY = 0,
    EZRA_FAULT_INTEGRITY = 1,
    EZRA_FAULT_SYSTEM = 2,
    EZRA_FAULT_MISSION = 3,
    EZRA_FAULT_TRUTHFULNESS = 4,
    EZRA_FAULT_UNAUTHORIZED_CONDITION = 5
} ezra_fault_t;

typedef struct {
    int running;
    ezra_operational_state_t operational_state;
    ezra_fault_t preservation_cause;
    int preservation_cause_set;
    void (*module_poll)(void *context);
    void *module_poll_context;
} ezra_runtime_t;

/* Core runtime functions do not require any installed modules. */
void ezra_runtime_init(ezra_runtime_t *runtime);
int ezra_runtime_run(ezra_runtime_t *runtime);
void ezra_runtime_request_stop(void);
int ezra_runtime_report_fault(ezra_runtime_t *runtime, ezra_fault_t fault);
int ezra_runtime_human_release(ezra_runtime_t *runtime, int authorized_human_release);
ezra_operational_state_t ezra_runtime_operational_state(const ezra_runtime_t *runtime);
int ezra_runtime_normal_operations_allowed(const ezra_runtime_t *runtime);

#endif
