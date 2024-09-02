#include "depend.h"
#include "alloc.h"
#include "options.h"
#include "util.h"
#include "repo.h"
#include "srcinfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <json-c/json.h>

int parse_srcinfo_deps(struct srcinfo *srcinfo,
                struct stringlist **repo_deps, struct stringlist **repo_bdeps,
                struct stringlist **aur_deps, struct stringlist **aur_bdeps,
                struct stringlist **pgp_keys)
{
        struct stringlist *depends = srcinfo->depends;
        struct stringlist *makedepends = srcinfo->makedepends;
        if (depends->n != 0)
                printf("Dependencies: ");
        for (size_t i = 0; i < depends->n; i++) {
                char *dep = depends->list[i];

                /* check if the dependency is installed */
                alpm_pkg_t *installed_pkg = alpm_find_satisfier(installed_packages, dep);
                if (installed_pkg != NULL)
                        continue;

                printf("%s ", dep);
                /* no package in repos */
                if (alpm_find_dbs_satisfier(alpm_handle, sync_dbs, dep)
                                == NULL) {
                        stringlist_append(*aur_deps, dep);
                        printf("(AUR) ");
                } else {
                        stringlist_append(*repo_deps, dep);
                }
        }
        if (depends->n != 0)
                printf("\n");

        if (makedepends->n != 0)
                printf("Build dependencies: ");

        for (size_t i = 0; i < makedepends->n; i++) {
                char *dep = makedepends->list[i];

                /* check if the dependency is installed */
                alpm_pkg_t *installed_pkg = alpm_find_satisfier(installed_packages, dep);
                if (installed_pkg != NULL)
                        continue;

                printf("%s ", dep);
                /* no package in repos */
                if (alpm_find_dbs_satisfier(alpm_handle, sync_dbs, dep)
                                == NULL) {
                        stringlist_append(*aur_bdeps, dep);
                        printf("(AUR) ");
                } else {
                        stringlist_append(*repo_bdeps, dep);
                }
        }
        if (makedepends->n != 0)
                printf("\n");

        struct stringlist *keylist = srcinfo->keylist;
        if (keylist->n != 0)
                printf("PGP keys: ");
        for (size_t i = 0; i < keylist->n; i++) {
                printf("%s ", keylist->list[i]);
        }
        if (keylist->n != 0)
                printf("\n");

        stringlist_concat(*pgp_keys, srcinfo->keylist);

        return 0;
}
