#include "srcinfo.h"
#include "options.h"
#include "alloc.h"
#include "string.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

/* key value pair for parsing srcinfo */
struct kv_pair {
        char *key;
        char *val;
};

static struct kv_pair *parse_kv_pair(const char *line)
{
        if (line == NULL)
                return NULL;

        /* TODO: clean this up... */
        char *dup = safe_strdup(line);

        int split_idx = first_char_idx(dup, '=');

        char *key_r = safe_calloc(split_idx + 32, 1);
        strncpy(key_r, dup, split_idx);
        char *key = trim_whitespace(key_r);
        free(key_r);

        char *val = trim_whitespace(dup + split_idx + 1);

        struct kv_pair *kvp = safe_calloc(1, sizeof(struct kv_pair));
        kvp->key = key;
        kvp->val = val;

        free(dup);

        return kvp;
}

static void free_kv_pair(struct kv_pair *kvp)
{
        if (kvp == NULL)
                return;

        free(kvp->key);
        free(kvp->val);
        free(kvp);
}

/* package sources must be downloaded at $cache_path/$pkg_name */
struct srcinfo *parse_pkg_srcinfo(const char *pkg_name)
{
        char *srcinfo_path = path_join(cache_path, pkg_name);
        if (srcinfo_path == NULL)
                return NULL;
        strcat(srcinfo_path, "/.SRCINFO");
        FILE *fp = fopen(srcinfo_path, "rb");
        if (!fp)
                fatal_err("failed to parse .SRCINFO for package %s", pkg_name);

        struct srcinfo *srcinfo = safe_calloc(1, sizeof(struct srcinfo));
        srcinfo->keylist = stringlist_new(8);
        srcinfo->depends = stringlist_new(64);
        srcinfo->makedepends = stringlist_new(64);

        char buf[2048];
        while (fgets(buf, 2048, fp) != NULL) {
                char *stripped = trim_whitespace(buf);
                if (strcmp(stripped, "") == 0)
                        continue;

                struct kv_pair *kvp = parse_kv_pair(stripped);
                free(stripped);

                if (strcmp(kvp->key, "depends") == 0) {
                        stringlist_append(srcinfo->depends, kvp->val);
                } else if (strcmp(kvp->key, "makedepends") == 0) {
                        stringlist_append(srcinfo->makedepends, kvp->val);
                } else if (strcmp(kvp->key, "validpgpkeys") == 0) {
                        stringlist_append(srcinfo->keylist, kvp->val);
                } else if (strcmp(kvp->key, "pkgbase") == 0) {
                        srcinfo->pkgbase = safe_strdup(kvp->val);
                } else if (strcmp(kvp->key, "pkgname") == 0) {
                        srcinfo->pkgname = safe_strdup(kvp->val);
                } else if (strcmp(kvp->key, "pkgver") == 0) {
                        srcinfo->pkgver = safe_strdup(kvp->val);
                }

                free_kv_pair(kvp);
        }

        return srcinfo;
}
