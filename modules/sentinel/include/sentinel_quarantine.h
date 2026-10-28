#ifndef EZRA_SENTINEL_QUARANTINE_H
#define EZRA_SENTINEL_QUARANTINE_H
#include <stddef.h>
typedef struct {
    unsigned int new_items;
    char example[128];
    char *snapshot;
    size_t snapshot_size;
} sentinel_quarantine_diff_t;
int sentinel_quarantine_check(const char *directory, const char *state,
                              sentinel_quarantine_diff_t *result);
int sentinel_quarantine_commit(sentinel_quarantine_diff_t *result, const char *state);
void sentinel_quarantine_clear(sentinel_quarantine_diff_t *result);
#endif
