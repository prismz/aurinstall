#include "new.h"
#include "util.h"
#include "repo.h"
#include "options.h"

#include <alpm.h>
#include <stdio.h>
#include <json-c/json.h>

static struct target_list *get_target_list(char **explicit_targets, int n)
{
        for (int i = 0; i < n; i++) {
                /* TODO: depmod operators for AUR packages */
                char *target_name = explicit_targets[i];
                json_object *meta = get_aur_pkg_meta(target_name);
                if (meta == NULL) {
                        /* search in repos */
                        alpm_pkg_t *repo_satisfier = alpm_find_dbs_satisfier(
                                        alpm_handle, sync_dbs, target_name);
                        if (repo_satisfier == NULL)
                                fatal_err("couldn't find package satisfying %s", target_name);

                }

        }
}

int n_install_packages(char **targets, int n)
{
        for (int i = 0; i < n; i++) {

        }
}
