#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <stdlib.h>

typedef struct { uid_t ruid, euid, suid; gid_t rgid, egid, sgid; mode_t umask; } Config;
int meta_exists(char[PATH_MAX]);
int get_meta_path(char[PATH_MAX], char[PATH_MAX]);
int read_meta_file(char[PATH_MAX], mode_t*, uid_t*, gid_t*, Config*);
int write_meta_file(char[PATH_MAX], mode_t, uid_t, gid_t, bool, Config*);
int dtoo(int), otod(int);

static int fails = 0;
#define CHECK(cond, msg, ...) do { if(cond){ printf("  OK   " msg "\n", ##__VA_ARGS__);} else { printf("  FAIL " msg "\n", ##__VA_ARGS__); fails++; } } while(0)

int main(int argc, char** argv) {
    char path[PATH_MAX]; strncpy(path, argv[1], PATH_MAX-1); path[PATH_MAX-1]=0;
    char meta[PATH_MAX];
    mode_t mode; uid_t owner; gid_t group;

    /* dtoo/otod inverse sanity for typical modes */
    CHECK(otod(dtoo(0700))==0700, "mode round-trip 0700 (got %o)", otod(dtoo(0700)));
    CHECK(otod(dtoo(0755))==0755, "mode round-trip 0755 (got %o)", otod(dtoo(0755)));
    CHECK(otod(dtoo(0600))==0600, "mode round-trip 0600 (got %o)", otod(dtoo(0600)));

    get_meta_path(path, meta);
    CHECK(strcmp(meta, path)==0, "get_meta_path returns the real path (no sidecar)");

    Config root = {0,0,0,0,0,0,022};       /* emulated root */
    Config pg   = {114,114,114,114,114,114,022}; /* emulated postgres (uid 114) */

    /* --- File with NO meta: per-caller euid fallback --- */
    CHECK(meta_exists(meta)!=0, "fresh file: meta_exists() == false");
    read_meta_file(meta,&mode,&owner,&group,&pg);
    CHECK(owner==114, "no-meta: stat-as-postgres sees euid 114 (fallback)");
    read_meta_file(meta,&mode,&owner,&group,&root);
    CHECK(owner==0,   "no-meta: stat-as-root sees euid 0 (fallback)");

    /* === Simulate pg_createcluster === */
    /* 1) mkdir as ROOT (handle_mk -> write_meta with creator euid=0) */
    write_meta_file(meta, 0700, root.euid, root.egid, true, &root);
    CHECK(meta_exists(meta)==0, "after mkdir-as-root: meta_exists() == true");
    read_meta_file(meta,&mode,&owner,&group,&pg);
    CHECK(owner==0, "after mkdir-as-root: owner persisted as 0 (root)");

    /* 2) chown postgres (USERLAND chown: read existing meta, rewrite owner) */
    read_meta_file(meta,&mode,&owner,&group,&root);          /* read current (mode 0700, owner 0) */
    write_meta_file(meta, mode, 114, 114, false, &root);     /* euid==0 may chown -> owner=114 */

    /* 3) THE KEY CHECKS: ownership is now PERSISTENT regardless of caller */
    read_meta_file(meta,&mode,&owner,&group,&pg);
    CHECK(owner==114 && group==114, "stat-as-postgres sees owner 114  (initdb check passes)");
    read_meta_file(meta,&mode,&owner,&group,&root);
    CHECK(owner==114 && group==114, "stat-as-ROOT sees owner 114      (pg_ctlcluster: 'not root' passes!)");
    CHECK(mode==0700, "mode preserved across chown (got %o)", mode);

    /* regression: write_meta on a not-yet-existent path (the mkdir/creat ENTER
     * case) must be NON-FATAL — else handle_mk propagates -1 == EPERM and the
     * guest mkdir fails ("install: cannot create directory: Operation not permitted"). */
    {
        char nope[PATH_MAX]; strcpy(nope, "/no/such/dir/deadbeef/x");
        int rc = write_meta_file(nope, 0755, 0, 0, true, &root);
        CHECK(rc == 0, "write_meta on non-existent path is non-fatal (rc=%d)", rc);
    }

    /* SSL-key scenario (the exit-time create-meta flush): a key created by root
     * with mode 0640 must report owner=root + 0640, NOT the loosened real mode
     * or the caller's euid. PostgreSQL rejects a db-user-owned key with g/o bits. */
    {
        char k[PATH_MAX]; strcpy(k, "/tmp"); /* placeholder, real path set below */
        /* simulate: open(O_CREAT,0640) as root -> EXIT flush writes meta(0640, root) */
        write_meta_file(meta, 0640, root.euid, root.egid, true, &root);
        read_meta_file(meta,&mode,&owner,&group,&pg);   /* postgres reads the key */
        CHECK(owner==0, "ssl key created-as-root: stat-as-postgres sees owner=root");
        CHECK((mode & 0777)==0640, "ssl key: mode is 0640, not loosened (got %o)", mode & 0777);
        (void)k;
    }

    printf("\n%s (%d failures)\n", fails? "*** TESTS FAILED ***":"ALL TESTS PASSED", fails);
    return fails?1:0;
}
