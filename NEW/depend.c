#include "depend.h"
#include "alloc.h"
#include "options.h"
#include "util.h"
#include "repo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

/* TODO: this could all definitely be improved.
 * I forgot alpm_checkdeps() exists... */

struct deplist *deplist_new(size_t cap)
{
        struct deplist *dl = safe_calloc(1, sizeof(struct deplist));

        dl->cap = cap;
        dl->n = 0;
        dl->dl = safe_calloc(cap, sizeof(struct dep *));

        return dl;
}

struct dep *satisfy_dep(const char *depstring, struct opts *opts, bool is_bdep)
{
        struct dep *d = safe_calloc(1, sizeof(struct dep));

        alpm_pkg_t *pkg = alpm_find_dbs_satisfier(opts->alpm_handle, opts->sync_dbs, depstring);
        /* invalid dependency, search AUR */
        if (pkg == NULL) {
                /* TODO: depmod operators */
                json_object *aur_meta = get_aur_pkg_meta(depstring, opts);
                if (aur_meta == NULL) {
                        warning("unsatisfiable dependency %s", depstring);
                        return NULL;
                }

                json_object *name_json = NULL;
                json_object_object_get_ex(aur_meta, "Name", &name_json);
                if (name_json == NULL) {
                        warning("unsatisfiable dependency %s", depstring);
                        return NULL;
                }

                d->satisfier = safe_strdup(json_object_get_string(name_json));
                d->is_aur = true;
                d->repo = safe_strdup("aur");
                d->depstring = safe_strdup(depstring);
                d->is_bdep = is_bdep;
                d->pkg = NULL;

                return d;
        }

        const char *satisfier = alpm_pkg_get_name(pkg);
        alpm_db_t *repo = alpm_pkg_get_db(pkg);
        const char *repo_name = alpm_db_get_name(repo);

        d->depstring = safe_strdup(depstring);
        d->satisfier = safe_strdup(satisfier);
        d->repo = safe_strdup(repo_name);
        d->is_bdep = is_bdep;
        d->is_aur = false;
        d->pkg = pkg;

        return d;
}

bool dep_satisfied(const char *depstring, struct opts *opts)
{
        alpm_pkg_t *pkg = alpm_find_satisfier(opts->installed_packages, depstring);
        if (pkg == NULL)
                return false;

        return true;
}

static int _parse_deplist(json_object *deps, bool bdeps, struct deplist *dl,
                struct opts *opts)
{
        int n = json_object_array_length(deps);
        for (int i = 0; i < n; i++) {
                json_object *dep_json = json_object_array_get_idx(deps, i);
                const char *depstring = json_object_get_string(dep_json);

                if (dep_satisfied(depstring, opts))
                        continue;

                struct dep *satisfier = satisfy_dep(depstring, opts, bdeps);

                /*
                bool duplicate = false;
                for (size_t j = 0; j < dl->n; j++) {
                        if (strcmp(dl->dl[j]->satisfier, satisfier->satisfier) == 0) {
                                duplicate = true;
                                break;
                        }
                }

                if (duplicate)
                        continue;
                */

                if (dl->n + 1 > dl->cap) {
                        dl->dl = safe_realloc(dl->dl, sizeof(struct dep *)
                                        * (dl->cap += 32));
                }

                if (satisfier->is_aur)
                        get_package_dependencies(satisfier->satisfier, dl, opts);

                dl->dl[dl->n++] = satisfier;
        }

        return 0;
}

static int parse_deplist(json_object *deps, struct deplist *dl,
                struct opts *opts)
{
        return _parse_deplist(deps, false, dl, opts);
}

static int parse_bdeplist(json_object *deps, struct deplist *dl,
                struct opts *opts)
{
        return _parse_deplist(deps, true, dl, opts);
}

int get_package_dependencies(const char *package_name, struct deplist *dl,
                struct opts *opts)
{
        json_object *meta = get_aur_pkg_meta(package_name, opts);
        /* package not in AUR, user should let pacman handle it */
        if (meta == NULL)
                return 0;

        json_object *deps_json;   /* dependencies */
        json_object *bdeps_json;  /* build dependencies */

        json_bool deps_avail = json_object_object_get_ex(meta,
                        "Depends", &deps_json);
        json_bool bdeps_avail = json_object_object_get_ex(meta,
                        "MakeDepends", &bdeps_json);

        if (deps_avail > 0) {
                if (parse_deplist(deps_json, dl, opts))
                        fatal_err("failed to parse dependencies");
        }

        if (bdeps_avail > 0) {
                if (parse_bdeplist(bdeps_json, dl, opts))
                        fatal_err("failed to parse build dependencies");
        }

        return 0;
}

/*
 * Returns AUR packages to install (in order!)
 * Sets *n to the number of these AUR packages to install.
 * Sets deps to all the dependencies that need to be installed,
 * this is used later to prompt for removal of build dependencies.
 *
 * This function exists for cases where an AUR package has AUR dependencies.
 */
char **install_dependencies(const char **targets, int n_targets, int *n,
                struct deplist *deps, struct opts *opts)
{
        char **aur_targets = safe_calloc(n_targets * 2, sizeof(char *));
        int n_aur_targets = 0;

        size_t dep_cmd_size = 4096;
        char *dep_cmd = safe_calloc(dep_cmd_size, 1);
        snprintf(dep_cmd, dep_cmd_size, "%s pacman -S ", opts->root_program);
        size_t dep_cmd_len = strlen(dep_cmd);

        for (int i = 0; i < n_targets; i++) {
                const char *target = targets[i];
                get_package_dependencies(target, deps, opts);
        }

        /* build command to install dependencies */
        for (size_t i = 0; i < deps->n; i++) {
                struct dep *dep = deps->dl[i];
                if (dep->is_aur) {
                        aur_targets[n_aur_targets++] = dep->satisfier;
                        continue;
                }

                size_t satisfier_len = strlen(dep->satisfier);

                if (dep_cmd_len + satisfier_len + 1 > dep_cmd_size)
                        dep_cmd = safe_realloc(dep_cmd, dep_cmd_size += 512);

                strcat(dep_cmd, dep->satisfier);
                strcat(dep_cmd, " ");
        }

        if (n != NULL)
                *n = n_aur_targets;

        if (system(dep_cmd)) {
                free(dep_cmd);
                return NULL;
        }

        free(dep_cmd);

        return aur_targets;
}
