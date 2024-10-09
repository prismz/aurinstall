#include "fastdeps.h"
#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <json-c/json.h>

json_object *db;

/* on Arch integrate this into alpm as done before, defaults to false
 * because this program is being developed on Debian... */
bool dep_satisfied()
{
        return false;
}

int format_repo(const char *path, const char *output)
{
        json_object *unformatted = json_object_from_file(path);
        if (unformatted == NULL)
                return 1;
        size_t unformatted_n = json_object_array_length(unformatted);

        json_object *formatted = json_object_new_object();
        for (size_t i = 0; i < unformatted_n; i++) {
                json_object *ent = json_object_array_get_idx(unformatted, i);

                json_object *name_json;
                json_object_object_get_ex(ent, "Name", &name_json);
                if (name_json == NULL)
                        continue;
                const char *name = json_object_get_string(name_json);

                json_object_object_add(formatted, name, ent);
        }

        if (json_object_to_file(output, formatted))
                return 1;

        return 0;
}

Target *target_new(const char *name)
{
        if (name == NULL)
                return NULL;

        json_object *metadata = package_get_metadata(name);
        if (metadata == NULL) {
                Target *t = safe_malloc(sizeof(Target));

                t->name = safe_strdup(name);
                t->desc = NULL;
                t->version = NULL;
                t->aur = false;
                t->depends = NULL;
                t->makedepends = NULL;

                return t;
        }

        json_object *desc_json;
        json_object *vers_json;
        json_object *depends_json;
        json_object *makedepends_json;

        json_object_object_get_ex(metadata, "Description", &desc_json);
        json_object_object_get_ex(metadata, "Version", &vers_json);
        json_object_object_get_ex(metadata, "Depends", &depends_json);
        json_object_object_get_ex(metadata, "MakeDepends", &makedepends_json);

        const char *desc = json_object_get_string(desc_json);
        const char *vers = json_object_get_string(vers_json);

        TargetList *depends = parse_deplist(depends_json);
        TargetList *makedepends = parse_deplist(makedepends_json);

        Target *t = safe_malloc(sizeof(Target));

        t->name = safe_strdup(name);
        t->desc = safe_strdup(desc);
        t->version = safe_strdup(vers);
        t->aur = true;

        t->depends = depends;
        t->makedepends = makedepends;

        return t;
}

TargetList *targetlist_new(size_t capacity)
{
        TargetList *tl = safe_malloc(sizeof(TargetList));

        tl->targets = safe_calloc(capacity, sizeof(Target *));
        tl->capacity = capacity;
        tl->n = 0;

        return tl;
}

void target_free(Target *t)
{
        if (t == NULL)
                return;
        free(t->name);
        free(t->desc);
        free(t->version);

        targetlist_free(t->depends);
        targetlist_free(t->makedepends);

        free(t);
}

void targetlist_free(TargetList *tl)
{
        if (tl == NULL)
                return;

        for (size_t i = 0; i < tl->n; i++)
                target_free(tl->targets[i]);
        free(tl->targets);
        free(tl);
}

int targetlist_append(TargetList *tl, Target *t)
{
        if (tl == NULL || tl->targets == NULL || t == NULL)
                return 1;

        if (tl->n + 1 > tl->capacity) {
                tl->capacity += 12;
                tl->targets = safe_realloc(tl->targets,
                                sizeof(Target *) * tl->capacity);
        }

        tl->targets[tl->n++] = t;

        return 0;
}

TargetList *parse_deplist(json_object *deps)
{
        if (deps == NULL)
                return NULL;

        size_t n_deps = json_object_array_length(deps);
        printf("parsing %ld deps\n", n_deps);
        if (n_deps == 0)
                return NULL;

        TargetList *tl = targetlist_new(n_deps * 4);

        for (size_t i = 0; i < n_deps; i++) {
                json_object *ent = json_object_array_get_idx(deps, i);
                const char *ent_name = json_object_get_string(ent);

                printf("%s", ent_name);
                if (dep_satisfied(ent_name)) {
                        printf(" (satisfied)\n");
                        continue;
                }
                printf("\n");

                Target *t = target_new(ent_name);
                targetlist_append(tl, t);
        }

        return tl;
}

json_object *package_get_metadata(const char *name)
{
        if (name == NULL)
                return NULL;

        json_object *result;
        json_object_object_get_ex(db, name, &result);
        if (result == NULL)
                return NULL;

        return result;
}

TargetList *get_full_targets(const char **names, int n)
{
        if (names == NULL || n < 1)
                return NULL;

        TargetList *tl = targetlist_new(n);

        for (int i = 0; i < n; i++) {
                const char *name = names[i];
                if (name == NULL)
                        break;

                Target *t = target_new(name);

                if (targetlist_append(tl, t)) {
                        targetlist_free(tl);
                        return NULL;
                }
        }

        return tl;
}

int main(void)
{
        clock_t begin = clock();

        /* only needs to be done once - saves to disk with better formatting for speed */
        /* if (format_repo("packages-meta-ext-v1.json", "aur-db.json"))
                return 1; */

        db = json_object_from_file("aur-db.json");
        if (db == NULL)
                return 1;

        double elapsed = time_since(begin);

        printf("loaded AUR database in %.3lfs\n", elapsed);

        clock_t search_begin_time = clock();
        Target *t = target_new("osu-lazer-bin");
        if (t == NULL)
                return 2;

        double search_elapsed = time_since(search_begin_time);
        printf("Metadata found in %lfs\n", search_elapsed);
}
