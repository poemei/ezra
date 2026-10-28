#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sentinel_quarantine.h"
static void require(int ok, const char *name)
{
    if (!ok) { fprintf(stderr, "FAIL: %s\n", name); exit(1); }
}
static void touch(const char *path)
{
    FILE *fp = fopen(path, "wb");
    require(fp != NULL, "create fixture");
    require(fclose(fp) == 0, "close fixture");
}
int main(void)
{
    char root[] = "/tmp/ezra-sentinel-test-XXXXXX";
    char dir[512], state[512], a[512], b[512], linkname[512];
    sentinel_quarantine_diff_t r;
    require(mkdtemp(root) != NULL, "temporary test root");
    snprintf(dir, sizeof dir, "%s/quarantine", root);
    snprintf(state, sizeof state, "%s/checkpoint", root);
    snprintf(a, sizeof a, "%s/old", dir);
    snprintf(b, sizeof b, "%s/new", dir);
    snprintf(linkname, sizeof linkname, "%s/symlink", dir);
    require(mkdir(dir, 0700) == 0, "make quarantine directory");
    touch(a);
    require(sentinel_quarantine_check(dir, state, &r) && r.new_items == 0, "initial inventory is baseline");
    require(sentinel_quarantine_commit(&r, state), "persist initial inventory");
    require(sentinel_quarantine_check(dir, state, &r) && r.new_items == 0, "unchanged inventory silent");
    require(sentinel_quarantine_commit(&r, state), "persist silent review");
    touch(b);
    require(sentinel_quarantine_check(dir, state, &r) && r.new_items == 1, "new file found");
    require(!strcmp(r.example, "new"), "new file named");
    sentinel_quarantine_clear(&r); /* Simulated failed report: do not acknowledge. */
    require(sentinel_quarantine_check(dir, state, &r) && r.new_items == 1, "failed delivery retries");
    require(sentinel_quarantine_commit(&r, state), "acknowledge after report");
    require(sentinel_quarantine_check(dir, state, &r) && r.new_items == 0, "acknowledged file quiet");
    require(sentinel_quarantine_commit(&r, state), "persist unchanged");
    require(symlink("/etc/passwd", linkname) == 0, "create symlink fixture");
    require(sentinel_quarantine_check(dir, state, &r) && r.new_items == 0, "symlink ignored");
    require(sentinel_quarantine_commit(&r, state), "persist symlink review");
    unlink(linkname); unlink(b); unlink(a); unlink(state); rmdir(dir); rmdir(root);
    puts("sentinel quarantine tests: PASS (baseline, new, retry, dedup, symlink)");
    return 0;
}
