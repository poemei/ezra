#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "stnlabz_abi/module.h"
#include "ezra_transport.h"
#include "sentinel_quarantine.h"

#define SENTINEL_LOG "/var/log/sentinel/sentinel.log"
#define SENTINEL_STATE "./state/sentinel.state"
#define SENTINEL_STATE_TMP "./state/sentinel.state.tmp"
#define INTERVAL_SECONDS (4U * 60U * 60U)
#define MAX_LINE 2048

typedef struct {
    unsigned long long offset;
    unsigned long long inode;
    unsigned long long device;
    unsigned long entries, processes, coverage;
    int known;
    int initial_sent;
} checkpoint_t;

typedef struct {
    unsigned int warnings, file_changes, process_exits, coverage_failures;
    unsigned long entries, processes, coverage;
    int scan_found;
} observations_t;

static pthread_t worker;
static atomic_int stopping;
static int running;
static const stnlabz_module_host_t *host;

static int volatile_path(const char *line)
{
    return strstr(line, "HOST_FILE_CHANGED /run/") ||
           strstr(line, "HOST_FILE_CHANGED /var/log/") ||
           strstr(line, "HOST_FILE_CHANGED /tmp/");
}

static int parse_scan(const char *line, observations_t *obs)
{
    const char *p = strstr(line, "HOST_SCAN_COMPLETE entries=");
    unsigned long e, n, c;
    if (!p || sscanf(p, "HOST_SCAN_COMPLETE entries=%lu processes=%lu coverage_errors=%lu",
                     &e, &n, &c) != 3) return 0;
    obs->entries = e; obs->processes = n; obs->coverage = c;
    obs->scan_found = 1;
    return 1;
}

static void classify(const char *line, observations_t *obs, int changes)
{
    if (parse_scan(line, obs)) {
        if (changes && obs->coverage) ++obs->coverage_failures;
        return;
    }
    if (!changes) return;
    if (strstr(line, "] WARNING:") || strstr(line, "] WARN:") ||
        strstr(line, "] ERROR:") || strstr(line, "] CRITICAL:"))
        ++obs->warnings;
    else if (strstr(line, "HOST_FILE_CHANGED ") && !volatile_path(line))
        ++obs->file_changes;
    else if (strstr(line, "HOST_PROCESS_EXIT "))
        ++obs->process_exits;
}

static int load_checkpoint(checkpoint_t *cp)
{
    FILE *fp = fopen(SENTINEL_STATE, "r");
    unsigned int sent, known;
    if (!fp) return 0;
    if (fscanf(fp, "v1 %llu %llu %llu %lu %lu %lu %u %u",
               &cp->offset, &cp->inode, &cp->device, &cp->entries,
               &cp->processes, &cp->coverage, &known, &sent) != 8) {
        fclose(fp);
        return 0;
    }
    cp->known = (int)known; cp->initial_sent = (int)sent;
    fclose(fp);
    return 1;
}

static int save_checkpoint(const checkpoint_t *cp)
{
    FILE *fp = fopen(SENTINEL_STATE_TMP, "w");
    if (!fp) return 0;
    if (fprintf(fp, "v1 %llu %llu %llu %lu %lu %lu %u %u\n",
                cp->offset, cp->inode, cp->device, cp->entries,
                cp->processes, cp->coverage,
                (unsigned int)cp->known, (unsigned int)cp->initial_sent) < 0 ||
        fflush(fp) != 0 || fsync(fileno(fp)) != 0 || fclose(fp) != 0) {
        (void)unlink(SENTINEL_STATE_TMP);
        return 0;
    }
    if (rename(SENTINEL_STATE_TMP, SENTINEL_STATE) != 0) return 0;
    return 1;
}

static int send_report(const char *text)
{
    ezra_transport_channel_request_t req;
    size_t len;
    if (!host || !host->invoke_service || !text) return 0;
    len = strlen(text);
    if (!len || len >= sizeof req.message) return 0;
    memset(&req, 0, sizeof req);
    memcpy(req.message, text, len + 1);
    return host->invoke_service(EZRA_TRANSPORT_CHANNEL_SEND_SERVICE,
             &req, sizeof req, NULL, 0, NULL) == STNLABZ_MODULE_OK;
}

static int inspect(checkpoint_t *cp)
{
    FILE *fp;
    struct stat st;
    observations_t obs = {0};
    char line[MAX_LINE];
    char message[EZRA_TRANSPORT_MESSAGE_MAX];
    long pos;
    int initial = !cp->initial_sent;
    int changed = 0;
    int needs_report = 0;
    sentinel_quarantine_diff_t quarantine = {0};
    int quarantine_ready = 0;

    fp = fopen(SENTINEL_LOG, "r");
    if (!fp) {
        fputs("[SENTINEL] Cannot read Sentinel log; check service permissions\n", stderr);
        return 0;
    }
    if (fstat(fileno(fp), &st) != 0 || !S_ISREG(st.st_mode)) {
        fclose(fp);
        return 0;
    }
    if (!initial && cp->known && cp->inode == (unsigned long long)st.st_ino &&
        cp->device == (unsigned long long)st.st_dev &&
        cp->offset <= (unsigned long long)st.st_size) {
        if (fseeko(fp, (off_t)cp->offset, SEEK_SET) != 0) {
            fclose(fp); return 0;
        }
    } else if (!initial) {
        /* Rotation: the old cursor is not valid. Avoid replaying historical
           alerts from a newly opened file, but still inspect its latest scan. */
        cp->offset = 0;
        changed = 0;
    }
    while (fgets(line, sizeof line, fp)) {
        size_t n = strlen(line);
        if (!n || line[n-1] != '\n') {
            if (!feof(fp)) { fclose(fp); return 0; }
            /* Preserve incomplete final records until next poll. */
            break;
        }
        classify(line, &obs, !initial && cp->offset != 0);
        if (!initial && cp->offset != 0) changed = 1;
    }
    pos = ftell(fp);
    if (pos < 0 || ferror(fp)) { fclose(fp); return 0; }
    fclose(fp);

    if (initial) {
        if (!obs.scan_found) return 1;
        (void)snprintf(message, sizeof message,
            "[EZRA/SENTINEL] Initial baseline: entries=%lu processes=%lu coverage_errors=%lu. Monitoring active.",
            obs.entries, obs.processes, obs.coverage);
        needs_report = 1;
    } else {
        if (obs.warnings || obs.file_changes || obs.process_exits ||
            (obs.scan_found && obs.coverage && obs.coverage != cp->coverage)) {
            (void)snprintf(message, sizeof message,
                "[EZRA/SENTINEL] Changes in 4h review: warnings=%u monitored_files=%u process_exits=%u coverage_errors=%lu.",
                obs.warnings, obs.file_changes, obs.process_exits,
                obs.scan_found ? obs.coverage : cp->coverage);
            needs_report = 1;
        }
    }
    if (sentinel_quarantine_check("/var/log/sentinel/quarantine",
            "./state/sentinel.quarantine", &quarantine)) {
        quarantine_ready = 1;
        if (quarantine.new_items > 0) {
            size_t used = strlen(message);
            if (!needs_report) {
                (void)snprintf(message, sizeof message,
                    "[EZRA/SENTINEL] 4h review: new quarantine items=%u%s%s",
                    quarantine.new_items,
                    quarantine.example[0] ? " (example: " : "",
                    quarantine.example[0] ? quarantine.example : "");
                if (quarantine.example[0]) {
                    used = strlen(message);
                    if (used + 2 < sizeof message) strcat(message, ")");
                }
            } else {
                (void)snprintf(message + used, sizeof message - used,
                    " New quarantine items=%u.", quarantine.new_items);
            }
            needs_report = 1;
        }
    } else {
        fputs("[SENTINEL] Quarantine inventory unavailable; checkpoint withheld\n", stderr);
        return 0;
    }
    if (needs_report && !send_report(message)) {
        fputs("[SENTINEL] Report delivery unavailable; retrying next check\n", stderr);
        return 0;
    }
    if (quarantine_ready && !sentinel_quarantine_commit(&quarantine,
            "./state/sentinel.quarantine")) return 0;
    (void)changed;
    cp->offset = (unsigned long long)pos;
    cp->inode = (unsigned long long)st.st_ino;
    cp->device = (unsigned long long)st.st_dev;
    if (obs.scan_found) {
        cp->entries = obs.entries; cp->processes = obs.processes;
        cp->coverage = obs.coverage; cp->known = 1;
    }
    if (initial) cp->initial_sent = 1;
    if (!save_checkpoint(cp)) {
        fputs("[SENTINEL] Cannot persist read checkpoint\n", stderr);
        return 0;
    }
    return 1;
}

static void *monitor(void *arg)
{
    checkpoint_t cp = {0};
    unsigned int wait = 0;
    (void)arg;
    (void)load_checkpoint(&cp);
    while (!atomic_load(&stopping)) {
        if (wait == 0) {
            int ok = inspect(&cp);
            wait = ok ? INTERVAL_SECONDS : 60U;
        }
        sleep(1);
        if (wait) --wait;
    }
    return NULL;
}

static stnlabz_module_result_t qualify(stnlabz_module_qualification_result_t *q)
{
    observations_t o = {0};
    unsigned int pass = 0;
    const char *sample = "[2026-10-27 23:51:51] INFO: HOST_SCAN_COMPLETE entries=101071 processes=124 coverage_errors=0";
    if (!q) return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
#define CHECK(expr) do { if (expr) ++pass; } while (0)
    CHECK(parse_scan(sample, &o));
    CHECK(o.scan_found);
    CHECK(o.entries == 101071);
    CHECK(o.processes == 124);
    CHECK(o.coverage == 0);
    CHECK(!parse_scan("garbage", &o));
    classify("[t] INFO: HOST_FILE_CHANGED /var/log/auth.log", &o, 1);
    CHECK(o.file_changes == 0);
    classify("[t] INFO: HOST_FILE_CHANGED /etc/ssh/sshd_config", &o, 1);
    CHECK(o.file_changes == 1);
    classify("[t] INFO: HOST_PROCESS_EXIT 123", &o, 1);
    CHECK(o.process_exits == 1);
    classify("[t] WARNING: unusual activity", &o, 1);
    CHECK(o.warnings == 1);
    classify("[t] INFO: HOST_FILE_CHANGED /etc/passwd", &o, 0);
    CHECK(o.file_changes == 1);
    CHECK(volatile_path("HOST_FILE_CHANGED /run/temp"));
#undef CHECK
    memset(q, 0, sizeof *q);
    q->tests_executed = 12;
    q->tests_passed = pass;
    q->tests_failed = 12 - pass;
    q->negative_test_executed = 1;
    q->negative_test_passed = 1;
    return pass == 12 ? STNLABZ_MODULE_OK : STNLABZ_MODULE_ERR_QUALIFICATION;
}

static stnlabz_module_result_t start(const stnlabz_module_host_t *h)
{
    if (running) return STNLABZ_MODULE_ERR_INVALID_STATE;
    if (!h || !h->invoke_service || access(SENTINEL_LOG, R_OK) != 0 ||
        access("./state", W_OK) != 0) return STNLABZ_MODULE_ERR_START_FAILED;
    host = h;
    atomic_store(&stopping, 0);
    if (pthread_create(&worker, NULL, monitor, NULL) != 0) {
        host = NULL;
        return STNLABZ_MODULE_ERR_START_FAILED;
    }
    running = 1;
    return STNLABZ_MODULE_OK;
}
static stnlabz_module_result_t stop(void)
{
    if (!running) return STNLABZ_MODULE_ERR_INVALID_STATE;
    atomic_store(&stopping, 1);
    if (pthread_join(worker, NULL) != 0) return STNLABZ_MODULE_ERR_STOP_FAILED;
    running = 0;
    host = NULL;
    return STNLABZ_MODULE_OK;
}
static const stnlabz_module_descriptor_t descriptor = {
    "sentinel", "Ezra Sentinel Observer", 1, 1, 0,
    STNLABZ_MODULE_API_MAJOR, STNLABZ_MODULE_API_MINOR,
    qualify, start, stop
};
const stnlabz_module_descriptor_t *stnlabz_module_get_descriptor(void)
{
    return &descriptor;
}
