#include "depend.h"
#include "alloc.h"
#include "options.h"
#include "util.h"
#include "repo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
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

        struct dep *d = safe_calloc(1, sizeof(struct dep *));
        d->depstring = safe_strdup(package_name);
        d->satisfier = safe_strdup(d->depstring);
        d->is_aur = true;
        d->is_bdep = false;
        d->repo = NULL;

        if (dl->n + 1 > dl->cap)
                dl->dl = safe_realloc(dl->dl, sizeof(struct dep *)
                                * (dl->cap += 8));
        dl->dl[dl->n++] = d;

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
struct deplist *install_dependencies(const char **targets, int n_targets, int *n,
                struct deplist *deps, struct opts *opts)
{
        struct deplist *aur_targets = deplist_new(n_targets * 2);
        int n_aur_targets = 0;

        for (int i = 0; i < n_targets; i++) {
                const char *target = targets[i];
                get_package_dependencies(target, deps, opts);
        }

        /* build command to install dependencies */
        size_t argv_size = 2048;
        char **argv = safe_calloc(argv_size, sizeof(char *));
        argv[0] = opts->root_program;
        argv[1] = "pacman";
        argv[2] = "-S";
        argv[3] = "--asdeps";
        argv[4] = "--";
        size_t argv_i = 5;

        for (size_t i = 0; i < deps->n; i++) {
                struct dep *dep = deps->dl[i];
                if (dep->is_aur) {
                        if (aur_targets->n + 1 > aur_targets->cap)
                                aur_targets->dl = safe_realloc(aur_targets->dl,
                                                (aur_targets->cap += 8));

                        aur_targets->dl[n_aur_targets++] = dep;
                        continue;
                }

                if (argv_i + 1 > argv_size)
                        argv = safe_realloc(argv, sizeof(char *) *
                                        (argv_size += 512));

                argv[argv_i++] = dep->satisfier;
        }

        if (argv_i == 5) {
                for (int i = 0; i < n_targets; i++) {
                        struct dep *d = safe_calloc(1, sizeof(struct dep));
                        d->satisfier = safe_strdup(targets[i]);
                        d->depstring = NULL;
                        d->is_aur = true;
                        d->repo = NULL;
                        json_object *meta = get_aur_pkg_meta(targets[i], opts);

                        json_object *vers_json;
                        json_object_object_get_ex(meta, "Version", &vers_json);
                        const char *vers = json_object_get_string(vers_json);

                        d->vers = safe_strdup(vers);

                        if (aur_targets->n + 1 > aur_targets->cap) {
                                aur_targets->dl = safe_realloc(aur_targets->dl,
                                                sizeof(struct dep) * (aur_targets->n += 8));
                        }

                        aur_targets->dl[aur_targets->n++] = d;
                }

                *n = n_targets;
                return aur_targets;
        }

        if (n != NULL)
                *n = aur_targets->n;

        if (fork() == 0) {
                if (execvp(opts->root_program, argv) < 0) {
                        return NULL;
                }
        } else {
                int ret;
                wait(&ret);
                if (ret != 0)
                        return NULL;


                return aur_targets;
        }

        return aur_targets;
}
