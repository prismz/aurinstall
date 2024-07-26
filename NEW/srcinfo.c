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

/* assumed input has no leading or trailing whitespace */
static struct kv_pair *parse_kv_pair(const char *line)
{
        if (line == NULL)
                return NULL;

        char *dup = safe_strdup(line);
        char *key;
        char *val;
        char *ptr = strtok(dup, " =");
        if (ptr != NULL) {
                key = safe_strdup(ptr);
                ptr = strtok(NULL, " =");
                val = safe_strdup(ptr);
        } else {
                free(dup);
                return NULL;
        }
        free(dup);

        struct kv_pair *kvp = safe_calloc(1, sizeof(struct kv_pair));
        kvp->key = key;
        kvp->val = val;

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
struct srcinfo *get_pkg_srcinfo(const char *pkg_name)
{
        char *srcinfo_path = path_join(cache_path, pkg_name);
        if (srcinfo_path == NULL)
                return NULL;
        FILE *fp = fopen(srcinfo_path, "rb");
        if (!fp)
                fatal_err("failed to parse .SRCINFO for package %s", pkg_name);


        struct srcinfo *srcinfo = safe_calloc(1, sizeof(struct srcinfo));
        srcinfo.keys =

        char buf[2048];
        while (fgets(buf, 2048, fp) != NULL) {
                char *stripped = trim_whitespace(buf);
                struct kv_pair *kvp = parse_kv_pair(stripped);
                free(stripped);

                if (strcmp(kvp->key, "depends") == 0) {

                } else if (strcmp(kvp->key, "makedepends") == 0) {
                } else if (strcmp(kvp->key, "validpgpkeys") == 0) {
                } else if (strcmp(kvp->key, "pkgbase") == 0) {
                } else if (strcmp(kvp->key, "pkgname") == 0) {
                } else if (strcmp(kvp->key, "pkgver") == 0) {
                }
        }


}
