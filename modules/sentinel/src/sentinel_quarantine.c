#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sentinel_quarantine.h"

/* An inventory of names and identities only: never open quarantine payloads.
 * Hex encoding removes separators/control characters from persisted state. */
#define MAX_ITEMS 10000U
#define KEY_MAX 600U
static int make_key(const char *name, const struct stat *st, char out[KEY_MAX])
{
    static const char hex[] = "0123456789abcdef";
    size_t n = strlen(name), i;
    int k;
    if (!n || n > 255U) return 0;
    for (i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)name[i];
        out[2*i] = hex[c >> 4];
        out[2*i+1] = hex[c & 15];
    }
    k = snprintf(out + n*2, KEY_MAX-n*2, ":%llu:%llu",
        (unsigned long long)st->st_dev, (unsigned long long)st->st_ino);
    return k > 0 && (size_t)k < KEY_MAX-n*2;
}
static int cmp_key(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}
static int has_key(const char *list, const char *key)
{
    const char *p = list;
    size_t n = strlen(key);
    while (p && *p) {
        const char *end = strchr(p, '\n');
        if (!end) break;
        if ((size_t)(end-p) == n && !memcmp(p, key, n)) return 1;
        p = end + 1;
    }
    return 0;
}
void sentinel_quarantine_clear(sentinel_quarantine_diff_t *result)
{
    if (!result) return;
    free(result->snapshot);
    memset(result, 0, sizeof *result);
}
int sentinel_quarantine_check(const char *directory, const char *state,
                              sentinel_quarantine_diff_t *result)
{
    DIR *dir = NULL;
    struct dirent *ent;
    struct stat st;
    char (*keys)[KEY_MAX] = NULL;
    char *prior = NULL, *next = NULL;
    size_t count = 0, i, prior_size = 0, next_size = 0;
    int fd, baseline = 0, ok = 0;
    FILE *fp = NULL;
    if (!directory || !state || !result) return 0;
    memset(result, 0, sizeof *result);
    fp = fopen(state, "rb");
    if (!fp) {
        if (errno != ENOENT) goto done;
        baseline = 1;
    } else {
        if (fseek(fp, 0, SEEK_END) || (long)(prior_size = (size_t)ftell(fp)) < 0 ||
            prior_size > MAX_ITEMS * KEY_MAX || fseek(fp, 0, SEEK_SET)) goto done;
        prior = calloc(prior_size + 1U, 1U);
        if (!prior || fread(prior, 1, prior_size, fp) != prior_size) goto done;
        if (strncmp(prior, "v1\n", 3)) goto done;
    }
    dir = opendir(directory);
    if (!dir) goto done;
    fd = dirfd(dir);
    if (fd < 0) goto done;
    keys = calloc(MAX_ITEMS, sizeof *keys);
    if (!keys) goto done;
    errno = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;
        if (fstatat(fd, ent->d_name, &st, AT_SYMLINK_NOFOLLOW) != 0) goto done;
        if (!S_ISREG(st.st_mode)) continue;
        if (count == MAX_ITEMS || !make_key(ent->d_name, &st, keys[count])) goto done;
        if (!baseline && !has_key(prior + 3, keys[count])) {
            ++result->new_items;
            if (!result->example[0]) {
                size_t n = strlen(ent->d_name), j;
                if (n >= sizeof result->example) n = sizeof result->example - 1U;
                for (j = 0; j < n; ++j) {
                    unsigned char c = (unsigned char)ent->d_name[j];
                    result->example[j] = (c >= 0x20 && c < 0x7f) ? (char)c : '_';
                }
            }
        }
        ++count;
        errno = 0;
    }
    if (errno != 0) goto done;
    qsort(keys, count, sizeof *keys, cmp_key);
    next = calloc(4U + count * KEY_MAX, 1U);
    if (!next) goto done;
    memcpy(next, "v1\n", 3);
    next_size = 3;
    for (i = 0; i < count; ++i) {
        size_t n = strlen(keys[i]);
        memcpy(next + next_size, keys[i], n);
        next_size += n;
        next[next_size++] = '\n';
    }
    result->snapshot = next;
    result->snapshot_size = next_size;
    next = NULL;
    ok = 1;
done:
    if (dir) closedir(dir);
    if (fp) fclose(fp);
    free(keys); free(prior); free(next);
    if (!ok) sentinel_quarantine_clear(result);
    return ok;
}
int sentinel_quarantine_commit(sentinel_quarantine_diff_t *result, const char *state)
{
    char tmp[512];
    FILE *fp;
    int fd;
    if (!result || !result->snapshot || !state ||
        snprintf(tmp, sizeof tmp, "%s.tmp", state) >= (int)sizeof tmp) return 0;
    fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) return 0;
    fp = fdopen(fd, "wb");
    if (!fp) { close(fd); return 0; }
    if (fwrite(result->snapshot, 1, result->snapshot_size, fp) != result->snapshot_size ||
        fflush(fp) || fsync(fd)) { fclose(fp); unlink(tmp); return 0; }
    if (fclose(fp) || rename(tmp, state)) { unlink(tmp); return 0; }
    sentinel_quarantine_clear(result);
    return 1;
}
