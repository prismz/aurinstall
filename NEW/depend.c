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
        alpm_pkg_t *pkg = alpm_find_dbs_satisfier(opts->alpm_handle, opts->sync_dbs, depstring);
        if (pkg == NULL)
                goto aur;

        const char *name = alpm_pkg_get_name(pkg);

        alpm_pkg_t *installed = alpm_db_get_pkg(opts->localdb, name);
        if (installed != NULL)
                return true;
aur:
        installed = alpm_db_get_pkg(opts->localdb, depstring);
        if (installed != NULL)
                return true;
        return false;
}

static int _parse_deplist(json_object *deps, bool bdeps, struct deplist *dl,
                struct opts *opts)
{
        int n = json_object_array_length(deps);
        for (int i = 0; i < n; i++) {
                json_object *dep_json = json_object_array_get_idx(deps, i);
                const char *depstring = json_object_get_string(dep_json);
                struct dep *satisfier = satisfy_dep(depstring, opts, bdeps);

                bool duplicate = false;
                for (size_t j = 0; j < dl->n; j++) {
                        if (strcmp(dl->dl[j]->satisfier, satisfier->satisfier) == 0) {
                                duplicate = true;
                                break;
                        }
                }

                if (duplicate)
                        continue;

                if (dl->n + 1 > dl->cap) {
                        dl->dl = safe_realloc(dl->dl, sizeof(struct dep *)
                                        * (dl->cap += 32));
                }

                dl->dl[dl->n++] = satisfier;
                if (satisfier->is_aur)
                        get_package_dependencies(satisfier->satisfier, dl, opts);
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
        if (meta == NULL) {
                warning("couldn't get dependencies for AUR package %s",
                                package_name);
                return 1;
        }

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

/* only used in get_pacman_full_targets() */
static void _parse_pacman_output(const char *cmd, struct deplist *dl, bool is_bdep, struct opts *opts)
{
        FILE *fp = popen(cmd, "r");
        if (!fp)
                fatal_err("failed to get standard repo dependencies");

        char buf[1024];
        while (fgets(buf, 1024, fp) != NULL) {
                char *repo = NULL;
                char *name = NULL;
                char *vers = NULL;

                int i = 0;
                /* this is a little dumb, but i don't want to use scanf(). */
                char *bufdup = safe_strdup(buf);
                char *ptr = strtok(bufdup, " ");
                while (ptr != NULL && i <= 3) {
                        switch (i) {
                        case 0:
                                repo = safe_strdup(ptr);
                                ptr = strtok(NULL, " \n");
                                i++;
                                break;
                        case 1:
                                name = safe_strdup(ptr);
                                ptr = strtok(NULL, " \n");
                                i++;
                                break;
                        case 2:
                                vers = safe_strdup(ptr);
                                ptr = strtok(NULL, " \n");
                                i++;
                                break;
                        default:
                                break;
                        }
                }


                struct dep *d = safe_calloc(1, sizeof(struct dep));
                d->repo = repo;
                d->depstring = NULL;
                d->satisfier = name;
                d->is_aur = false;
                d->is_bdep = is_bdep;
                d->is_new_bdep = false;
                d->vers = vers;

                if (d->is_bdep) {
                        if (dep_satisfied(name, opts))
                                d->is_new_bdep = false;
                        else
                                d->is_new_bdep = true;
                }

                /* resize if needed */
                if (dl->n + 1 > dl->cap) {
                        dl->dl = safe_realloc(dl->dl, sizeof(struct dep *)
                                        * (dl->cap += 16));
                }

                dl->dl[dl->n++] = d;
        }

        pclose(fp);
}

/*
 * For all non-aur packages in dl, add them to a query for pacman.
 * Make pacman list all targets (the idea of this is that
 * pacman will resolve further dependencies on its own.
 */
static struct deplist *get_pacman_full_targets(struct deplist *dl, struct opts *opts)
{
        /* we have to separate build dependencies from normal dependencies
         * entirely and run two separate commands, so we can find what all
         * the build dependencies are.
         */

        size_t dep_cmd_size = 2048;
        char *dep_cmd = safe_calloc(dep_cmd_size, 1);
        snprintf(dep_cmd, dep_cmd_size, "pacman -p --print-format \"%%r %%n %%v\" -S ");
        size_t dep_cmd_i = strlen(dep_cmd) - 1;

        size_t bdep_cmd_size = 2048;
        char *bdep_cmd = safe_calloc(bdep_cmd_size, 1);
        snprintf(bdep_cmd, bdep_cmd_size, "pacman -p --print-format \"%%r %%n %%v\" -S ");
        size_t bdep_cmd_i = strlen(bdep_cmd) - 1;

        for (size_t i = 0; i < dl->n; i++) {
                struct dep *d = dl->dl[i];
                char *depname = d->satisfier;
                size_t depname_len = strlen(depname);

                if (d->is_aur)
                        continue;

                if (d->is_bdep) {
                        /* resize if needed */
                        if (bdep_cmd_i + depname_len + 18 >= bdep_cmd_size) {
                                bdep_cmd = safe_realloc(bdep_cmd,
                                                bdep_cmd_size
                                                += (depname_len + 64)
                                );
                        }

                        strcat(bdep_cmd, depname);
                        strcat(bdep_cmd, " ");
                        continue;
                }

                /* resize if needed */
                if (dep_cmd_i + depname_len + 18 >= dep_cmd_size)
                        dep_cmd = safe_realloc(dep_cmd, dep_cmd_size += (depname_len + 64));

                strcat(dep_cmd, depname);
                strcat(dep_cmd, " ");
        }

        /* redirect error output (sometimes no build dependencies) */
        strcat(dep_cmd, " 2> /dev/null");
        strcat(bdep_cmd, " 2> /dev/null");

        struct deplist *deps = deplist_new(dl->cap);
        struct deplist *bdeps = deplist_new(dl->cap);

        _parse_pacman_output(dep_cmd, deps, false, opts);
        _parse_pacman_output(bdep_cmd, bdeps, true, opts);

        struct deplist *full_deps = deplist_new(deps->n + bdeps->n + 12);

        /* no need to bounds check - pre allocated */
        for (int i = 0; i < deps->n; i++)
                full_deps->dl[full_deps->n++] = deps->dl[i];
        for (int i = 0; i < bdeps->n; i++)
                full_deps->dl[full_deps->n++] = bdeps->dl[i];

        free(deps);
        free(bdeps);

        return full_deps;
}

struct deplist *get_targets_dependencies(const char **targets, int n, struct opts *opts)
{
        struct deplist *dl = deplist_new(n * 16);
        for (int i = 0; i < n; i++)
                get_package_dependencies(targets[i], dl, opts);

        struct deplist *all_repo_deps = get_pacman_full_targets(dl, opts);

        struct deplist *all_deps = deplist_new(dl->n + all_repo_deps->n + 8);
        for (size_t i = 0; i < dl->n; i++) {
                struct dep *d = dl->dl[i];
                if (!d->is_aur)
                        continue;

                if (!dep_satisfied(d->satisfier, opts)) {
                        all_deps->dl[all_deps->n++] = d;
                        continue;
                }

                free(d->repo);
                free(d->depstring);
                free(d->satisfier);
                free(d);
        }

        for (size_t i = 0; i < all_repo_deps->n; i++) {
                struct dep *d = all_repo_deps->dl[i];
                if (!dep_satisfied(d->satisfier, opts)) {
                        all_deps->dl[all_deps->n++] = d;
                        continue;
                }

                free(d->repo);
                free(d->depstring);
                free(d->satisfier);
                free(d);
        }

        free(dl);
        free(all_repo_deps);

        return all_deps;
}

bool dependency_prompt(struct deplist *dl, struct opts *opts)
{
        printf("%ld deps\n", dl->n);
        for (size_t i = 0; i < dl->n; i++) {
                struct dep *d = dl->dl[i];
                printf("%s/%s %s", d->repo, d->satisfier, d->vers);
                printf("\n");
        }

        return false;
}
