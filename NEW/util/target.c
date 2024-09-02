#include "target.h"
#include "../options.h"
#include "util.h"
#include "repo.h"
#include "alloc.h"

#include <alpm.h>
#include <json-c/json.h>

struct target *target_new(const char *name, const char *ver, bool is_installed,
                bool is_explicit, bool is_makedep, bool in_repos,
                struct target_list *dependencies)
{
        struct target *t = safe_calloc(1, sizeof(struct target));

        t->name = safe_strdup(name);
        t->ver = safe_strdup(ver);
        t->is_installed = is_installed;
        t->is_explicit = is_explicit;
        t->is_makedep = is_makedep;
        t->in_repos = in_repos;
        if (dependencies != NULL)
                t->dependencies = dependencies;
        else
                t->dependencies = target_list_new(8);

        return t;
}

struct target_list *target_list_new(size_t capacity)
{
        struct target_list *tl = safe_calloc(1, sizeof(struct target_list));

        if (capacity == 0)
                tl->capacity = 8;
        else
                tl->capacity = capacity;
        tl->n = 0;
        tl->targets = safe_calloc(capacity, sizeof(struct target *));

        return tl;
}

void target_list_free(struct target_list *tl)
{
        if (tl == NULL)
                return;
        if (tl->targets == NULL) {
                free(tl);
                return;
        }

        for (int i = 0; i < tl->n; i++)
                target_free(tl->targets[i]);
        free(tl->targets);
        free(tl);
}

void target_free(struct target *t)
{
        free(t->name);
        free(t->ver);
        target_list_free(t->dependencies);
}

/* returns index of what was added */
int target_list_append(struct target_list *tl, struct target *t)
{
        if (tl->n + 1 > (long int)tl->capacity) {
                tl->targets = safe_realloc(tl->targets, (tl->capacity += 8)
                                * sizeof(struct target *));
        }

        tl->targets[tl->n++] = t;

        return tl->n - 1;
}

/* deplist should be a JSON array of JSON strings */
static int parse_deplist_to_targetlist(json_object *deplist, bool are_makedeps,
                struct target_list *dest)
{
        bool is_array = json_object_is_type(deplist, json_type_array);
        if (!is_array)
                return 1;

        int n_deps = json_object_array_length(deplist);
        for (int i = 0; i < n_deps; i++) {
                json_object *dep_json = json_object_array_get_idx(deplist, i);
                const char *depstring = json_object_get_string(dep_json);

                struct target *target = get_target_from_name(depstring, false,
                                are_makedeps);
                if (target == NULL)
                        fatal_err("failed to resolve target for %s\n", depstring);

                if (target->is_installed)
                        continue;

                target_list_append(dest, target);
        }

        return 0;
}

struct target *get_target_from_name(const char *target_name, bool is_explicit, bool is_makedep)
{
        if (target_name == NULL)
                return NULL;

        struct target *target;
        bool is_installed = package_is_installed(target_name);
        json_object *meta = get_aur_pkg_meta(target_name);
        if (meta == NULL) {
                /* search in repos */
                alpm_pkg_t *repo_satisfier = alpm_find_dbs_satisfier(
                                alpm_handle, sync_dbs, target_name);
                if (repo_satisfier == NULL)
                        fatal_err("couldn't find package satisfying %s", target_name);

                const char *satisfier_name, *satisfier_ver;
                satisfier_name = alpm_pkg_get_name(repo_satisfier);
                satisfier_ver = alpm_pkg_get_version(repo_satisfier);

                target = target_new(satisfier_name, satisfier_ver,
                                is_installed, is_explicit, is_makedep, true, NULL);

                return target;
        }

        json_object *ver_json, *makedep_json, *dep_json;
        json_object_object_get_ex(meta, "Version", &ver_json);
        json_object_object_get_ex(meta, "MakeDepends", &makedep_json);
        json_object_object_get_ex(meta, "Depends", &dep_json);

        const char *ver = json_object_get_string(ver_json);

        target = target_new(target_name, ver, is_installed, is_explicit,
                        is_makedep, false, NULL);

        parse_deplist_to_targetlist(makedep_json, true, target->dependencies);
        parse_deplist_to_targetlist(dep_json, false, target->dependencies);

        return target;
}

struct target_list *get_target_list(char **explicit_targets, int n)
{
        struct target_list *tl = target_list_new(n);

        for (int i = 0; i < n; i++) {
                /* TODO: depmod operators for AUR packages */
                char *target_name = explicit_targets[i];
                struct target *target = get_target_from_name(target_name, true,
                                false);
                target_list_append(tl, target);
        }

        return tl;
}

/* TODO: make output nicer */
void target_list_print(struct target_list *tl, int depth)
{
        if (tl == NULL) {
                printf("(NULL)\n");
                return;
        }

        for (int i = 0; i < tl->n; i++) {
                /* print indent */
                for (int j = 0; j < depth; j++)
                        printf("  | ");
                struct target *target = tl->targets[i];
                printf("%d%d%d%d %s %s\n", target->is_installed,
                                target->is_explicit,
                                target->is_makedep,
                                target->in_repos,
                                target->name,
                                target->ver);

                if (target->dependencies == NULL)
                        target->dependencies = target_list_new(8);

                target_list_print(target->dependencies, depth + 1);
        }
}
