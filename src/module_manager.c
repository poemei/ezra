#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "logging.h"
#include "module_manager.h"

static ezra_module_result_t recorded(const char *event, const char *id,
    ezra_module_result_t result)
{
    char detail[160];
    /* No admission or lifecycle decision may succeed without its audit trail. */
    if (!ezra_log_is_open()) return EZRA_MODULE_CALLBACK_FAILED;
    (void)snprintf(detail, sizeof detail, "id=%s result=%d",
                   id ? id : "unknown", (int)result);
    if (!ezra_log_event("MODULE", event, detail) || !ezra_log_sync())
        return EZRA_MODULE_CALLBACK_FAILED;
    return result;
}

/* Admission gate: reject malformed evidence before changing module state. */
static int valid_qualification(const ezra_module_qualification_t *q)
{
    return q && q->tests_executed >= EZRA_MODULE_MIN_TESTS &&
        q->tests_failed == 0 &&
        q->tests_passed == q->tests_executed &&
        q->negative_test_executed == 1 &&
        q->negative_test_passed == 1;
}

static int valid_id(const char *s)
{
    size_t i;
    if (!s) return 0;
    for (i = 0; i < EZRA_MODULE_ID_MAX; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!c) return i > 0;
        if (!(islower(c) || isezra(c) || c == '_' || c == '-')) return 0;
    }
    return 0;
}
static ezra_module_record_t *find_mut(ezra_module_manager_t *m, const char *id)
{
    size_t i;
    if (!m || !valid_id(id)) return NULL;
    for (i = 0; i < m->count; ++i)
        if (strcmp(m->modules[i].descriptor.id, id) == 0) return &m->modules[i];
    return NULL;
}
const ezra_module_record_t *ezra_module_find(const ezra_module_manager_t *m, const char *id)
{
    size_t i;
    if (!m || !valid_id(id)) return NULL;
    for (i = 0; i < m->count; ++i)
        if (strcmp(m->modules[i].descriptor.id, id) == 0) return &m->modules[i];
    return NULL;
}
void ezra_module_manager_init(ezra_module_manager_t *m)
{
    if (m) memset(m, 0, sizeof(*m));
    if (ezra_log_is_open()) (void)ezra_log_event("MODULE", "MANAGER_INIT", "registered=0");
}
ezra_module_result_t ezra_module_register(ezra_module_manager_t *m,
    const ezra_module_descriptor_t *d, const ezra_module_callbacks_t *cb)
{
    ezra_module_record_t *r;
    if (!m || !d || !cb || !valid_id(d->id) || !cb->start || !cb->stop ||
        d->abi_major != EZRA_MODULE_ABI_MAJOR ||
        d->abi_minor > EZRA_MODULE_ABI_MINOR) return recorded("REGISTER", d ? d->id : NULL, EZRA_MODULE_INVALID);
    if (find_mut(m, d->id)) return recorded("REGISTER", d ? d->id : NULL, EZRA_MODULE_DUPLICATE);
    if (m->count >= EZRA_MODULE_MAX) return recorded("REGISTER", d ? d->id : NULL, EZRA_MODULE_FULL);
    if (recorded("REGISTER", d->id, EZRA_MODULE_OK) != EZRA_MODULE_OK)
        return EZRA_MODULE_CALLBACK_FAILED;
    r = &m->modules[m->count++];
    memset(r, 0, sizeof(*r));
    r->descriptor = *d;
    r->callbacks = *cb;
    r->state = EZRA_MODULE_REGISTERED;
    return EZRA_MODULE_OK;
}
ezra_module_result_t ezra_module_qualify(ezra_module_manager_t *m,
    const char *id, const ezra_module_qualification_t *q)
{
    ezra_module_record_t *r = find_mut(m, id);
    if (!r) return recorded("QUALIFY", id, EZRA_MODULE_NOT_FOUND);
    if (r->state != EZRA_MODULE_REGISTERED || !valid_qualification(q))
        return recorded("QUALIFY", id, EZRA_MODULE_UNQUALIFIED);
    if (recorded("QUALIFY", id, EZRA_MODULE_OK) != EZRA_MODULE_OK)
        return EZRA_MODULE_CALLBACK_FAILED;
    r->qualification = *q;
    r->state = EZRA_MODULE_QUALIFIED;
    return EZRA_MODULE_OK;
}
ezra_module_result_t ezra_module_authorize(ezra_module_manager_t *m,
    const char *id, int operator_authorized)
{
    ezra_module_record_t *r = find_mut(m, id);
    if (!r) return recorded("AUTHORIZE", id, EZRA_MODULE_NOT_FOUND);
    if (r->state != EZRA_MODULE_QUALIFIED && r->state != EZRA_MODULE_STOPPED)
        return recorded("AUTHORIZE", id, EZRA_MODULE_UNQUALIFIED);
    if (operator_authorized != 1) {
        r->activation_authorized = 0;
        return recorded("AUTHORIZE", id, EZRA_MODULE_UNQUALIFIED);
    }
    if (!valid_qualification(&r->qualification))
        return recorded("AUTHORIZE", id, EZRA_MODULE_UNQUALIFIED);
    if (recorded("AUTHORIZE", id, EZRA_MODULE_OK) != EZRA_MODULE_OK)
        return EZRA_MODULE_CALLBACK_FAILED;
    r->activation_authorized = 1;
    return EZRA_MODULE_OK;
}
ezra_module_result_t ezra_module_activate(ezra_module_manager_t *m, const char *id)
{
    ezra_module_record_t *r = find_mut(m, id);
    if (!r) return recorded("ACTIVATE", id, EZRA_MODULE_NOT_FOUND);
    if ((r->state != EZRA_MODULE_QUALIFIED && r->state != EZRA_MODULE_STOPPED) ||
        !valid_qualification(&r->qualification) || !r->activation_authorized)
        return recorded("ACTIVATE", id, EZRA_MODULE_UNQUALIFIED);
    if (recorded("ACTIVATE_ATTEMPT", id, EZRA_MODULE_OK) != EZRA_MODULE_OK)
        return EZRA_MODULE_CALLBACK_FAILED;
    if (!r->callbacks.start(r->callbacks.context)) return recorded("ACTIVATE", id, EZRA_MODULE_CALLBACK_FAILED);
    r->state = EZRA_MODULE_ACTIVE;
    return recorded("ACTIVATE", id, EZRA_MODULE_OK);
}
ezra_module_result_t ezra_module_stop(ezra_module_manager_t *m, const char *id)
{
    ezra_module_record_t *r = find_mut(m, id);
    if (!r) return recorded("STOP", id, EZRA_MODULE_NOT_FOUND);
    if (r->state != EZRA_MODULE_ACTIVE) return recorded("STOP", id, EZRA_MODULE_INVALID);
    if (recorded("STOP_ATTEMPT", id, EZRA_MODULE_OK) != EZRA_MODULE_OK)
        return EZRA_MODULE_CALLBACK_FAILED;
    r->callbacks.stop(r->callbacks.context);
    r->state = EZRA_MODULE_STOPPED;
    r->activation_authorized = 0;
    return recorded("STOP", id, EZRA_MODULE_OK);
}
void ezra_module_manager_shutdown(ezra_module_manager_t *m)
{
    size_t i;
    if (!m) return;
    for (i = m->count; i > 0; --i) {
        ezra_module_record_t *r = &m->modules[i-1];
        if (r->state == EZRA_MODULE_ACTIVE) {
            r->callbacks.stop(r->callbacks.context);
            r->state = EZRA_MODULE_STOPPED;
            (void)recorded("SHUTDOWN_STOP", r->descriptor.id, EZRA_MODULE_OK);
        }
        r->activation_authorized = 0;
    }
    (void)recorded("MANAGER_SHUTDOWN", NULL, EZRA_MODULE_OK);
}
