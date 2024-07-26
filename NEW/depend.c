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

struct dep *satisfy_dep(const char *depstring, bool is_bdep)
{
        struct dep *d = safe_calloc(1, sizeof(struct dep));

        alpm_pkg_t *pkg = alpm_find_dbs_satisfier(alpm_handle, sync_dbs, depstring);
        /* invalid dependency, search AUR */
        if (pkg == NULL) {
                /* TODO: depmod operators */
                json_object *aur_meta = get_aur_pkg_meta(depstring);
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

                json_object *vers_json = NULL;
                json_object_object_get_ex(aur_meta, "Version", &vers_json);
                if (vers_json == NULL) {
                        warning("couldn't find version for AUR dependency %s", depstring);
                        return NULL;
                }

                d->satisfier = safe_strdup(json_object_get_string(name_json));
                d->is_aur = true;
                d->repo = safe_strdup("aur");
                d->vers = safe_strdup(json_object_get_string(vers_json));
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

bool dep_satisfied(const char *depstring)
{
        alpm_pkg_t *pkg = alpm_find_satisfier(installed_packages, depstring);
        if (pkg == NULL)
                return false;

        return true;
}

static int _parse_deplist(json_object *deps, bool bdeps, struct deplist *dl)
{
        int n = json_object_array_length(deps);
        for (int i = 0; i < n; i++) {
                json_object *dep_json = json_object_array_get_idx(deps, i);
                const char *depstring = json_object_get_string(dep_json);

                if (dep_satisfied(depstring))
                        continue;

                struct dep *satisfier = satisfy_dep(depstring, bdeps);

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
                        get_package_dependencies(satisfier->satisfier, dl);

                dl->dl[dl->n++] = satisfier;
        }

        return 0;
}

static int parse_deplist(json_object *deps, struct deplist *dl)
{
        return _parse_deplist(deps, false, dl);
}

static int parse_bdeplist(json_object *deps, struct deplist *dl)
{
        return _parse_deplist(deps, true, dl);
}

/* package sources must be installed */
int get_package_dependencies(const char *package_name, struct deplist *dl)
{
        /* TODO: remove meta file and clone packages and parse srcinfo instead */
        json_object *meta = get_aur_pkg_meta(package_name);
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
                if (parse_deplist(deps_json, dl))
                        fatal_err("failed to parse dependencies");
        }

        if (bdeps_avail > 0) {
                if (parse_bdeplist(bdeps_json, dl))
                        fatal_err("failed to parse build dependencies");
        }

        return 0;
}

/*
 * All targets should already be cloned.
 * Will clone AUR dependencies if needed.
 *
 * Returns AUR packages to install (in order!)
 * Sets *n to the number of these AUR packages to install.
 * Sets deps to all the dependencies that need to be installed,
 * this is used later to prompt for removal of build dependencies.
 *
 * This function exists for cases where an AUR package has AUR dependencies.
 */
struct deplist *install_dependencies(const char **targets, int n)
{
        struct deplist *aur_targets = deplist_new(n * 2);
        struct deplist *deps = deplist_new(n * 8);

        /* get dependencies of all packages into list */
        for (int i = 0; i < n; i++) {
                const char *target = targets[i];
                get_package_dependencies(target, deps);
        }

        /* build command to install dependencies */
        size_t argv_size = 2048;
        char **argv = safe_calloc(argv_size, sizeof(char *));
        argv[0] = root_program;
        argv[1] = "pacman";
        argv[2] = "-S";
        argv[3] = "--asdeps";
        argv[4] = "--";
        size_t argv_i = 5;

        /* build pacman command to install dependencies, while also storing
         * extra AUR dependencies */
        for (size_t i = 0; i < deps->n; i++) {
                struct dep *dep = deps->dl[i];
                if (dep->is_aur) {
                        if (aur_targets->n + 1 > aur_targets->cap)
                                aur_targets->dl = safe_realloc(aur_targets->dl,
                                                (aur_targets->cap += 8));

                        aur_targets->dl[aur_targets->n++] = dep;
                        aur_targets->dl[aur_targets->n - 1]->is_explicit = false;
                        continue;
                }

                if (argv_i + 1 > argv_size)
                        argv = safe_realloc(argv, sizeof(char *) *
                                        (argv_size += 512));

                argv[argv_i++] = dep->satisfier;
        }

        for (int i = 0; i < n; i++) {
                struct dep *d = safe_calloc(1, sizeof(struct dep));
                d->satisfier = safe_strdup(targets[i]);
                d->depstring = NULL;
                d->is_aur = true;
                d->repo = NULL;
                json_object *meta = get_aur_pkg_meta(targets[i]);

                json_object *vers_json;
                json_object_object_get_ex(meta, "Version", &vers_json);
                const char *vers = json_object_get_string(vers_json);

                d->vers = safe_strdup(vers);

                if (aur_targets->n + 1 > aur_targets->cap) {
                        aur_targets->dl = safe_realloc(aur_targets->dl,
                                        sizeof(struct dep) * (aur_targets->n += 8));
                }

                aur_targets->dl[aur_targets->n++] = d;
                aur_targets->dl[aur_targets->n - 1]->is_explicit = true;
        }

        if (argv_i == 5)
                return aur_targets;

        pid_t pid = fork();

        if (pid == -1) {
                fatal_err("failed to fork()");
        } else if (pid > 0) {
                int ret;
                waitpid(pid, &ret, 0);
                if (ret != 0)
                        fatal_err("failed to install dependencies");

                return aur_targets;

        } else {
                execvp(root_program, argv);
                fatal_err("failed to install dependencies");
        }

        return aur_targets;
}
