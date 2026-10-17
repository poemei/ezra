#ifndef EZRA_CORE_POLICY_H
#define EZRA_CORE_POLICY_H
#include "agent_root.h"
typedef enum {
    EZRA_POLICY_ALLOW = 0,
    EZRA_POLICY_DENY,
    EZRA_POLICY_ESCALATE
} ezra_policy_decision_t;
typedef struct {
    const char *actor;           /* Already verified by a trusted Core caller. */
    const char *mission;         /* Assigned mission identifier. */
    const char *operation;       /* Operation being requested. */
    const char *module_id;       /* Target module, if applicable. */
    int within_mission;
    int complies_with_policy;
    int instruction_authorized;
    int authority_granted;
    const char *escalation_target; /* Valid next-level contact supplied by operator policy. */
    const char *verification_source; /* Trusted Core caller provenance label. */
    int context_verified; /* Asserted by trusted caller, NOT independent authentication. */
} ezra_policy_request_t;
typedef enum {
    EZRA_ESCALATION_NONE = 0,
    EZRA_ESCALATION_PENDING_DELIVERY,
    EZRA_ESCALATION_UNAVAILABLE
} ezra_escalation_status_t;
typedef struct {
    ezra_policy_decision_t decision;
    const char *policy_id;
    const char *reason;
    ezra_escalation_status_t escalation_status; /* No external delivery is implied. */
} ezra_policy_result_t;
/* Fail-closed evaluation; writes a synchronized policy decision to the Core log. */
int ezra_core_policy_evaluate(const ezra_policy_request_t *request,
    ezra_policy_result_t *result);
#endif
