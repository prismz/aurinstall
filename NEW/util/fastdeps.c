#include "fastdeps.h"
#include "repo.h"
#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <json-c/json.h>

/* on Arch integrate this into alpm as done before, defaults to false
 * because this program is being developed on Debian... */

Target *target_new(const char *name)
{
        if (name == NULL)
                return NULL;

        json_object *metadata = get_aur_pkg_meta(name);
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
        //printf("parsing %ld deps\n", n_deps);
        if (n_deps == 0)
                return NULL;

        TargetList *tl = targetlist_new(n_deps * 4);

        for (size_t i = 0; i < n_deps; i++) {
                json_object *ent = json_object_array_get_idx(deps, i);
                const char *ent_name = json_object_get_string(ent);

                //printf("%s", ent_name);
                if (package_is_installed(ent_name)) {
                        //printf(" (satisfied)\n");
                        continue;
                }
                //printf("\n");

                Target *t = target_new(ent_name);
                targetlist_append(tl, t);
        }

        return tl;
}

TargetList *get_target_list(const char **target_names, int n)
{
        if (target_names == NULL || n < 1)
                return NULL;

        TargetList *tl = targetlist_new(n);

        for (int i = 0; i < n; i++) {
                const char *target_name = target_names[i];
                if (target_name == NULL)  /* break on NULL */
                        break;

                Target *t = target_new(target_name);
                if (t == NULL) {
                        fprintf(stderr, "Couldn't get information for package %s\n", target_name);
                        return NULL;
                }
                targetlist_append(tl, t);
        }

        return tl;
}

void _print_targetlist(TargetList *tl, int depth, bool are_makedepends)
{
        if (tl == NULL)
                return;
        for (int i = 0; i < tl->n; i++) {
                Target *t = tl->targets[i];

                for (int j = 0; j < depth; j++)
                        printf("  ");
                printf("|- %s", t->name);
                if (are_makedepends)
                        printf(" (Build Dependency)");
                printf("\n");

                if (t->depends && t->depends->n > 0)
                        _print_targetlist(t->depends, depth + 1, false);

                if (t->makedepends && t->makedepends->n > 0)
                        _print_targetlist(t->makedepends, depth + 1, true);
        }
}

#define print_targetlist(tl) (_print_targetlist(tl, 0, false))

int _main(void)
{
        clock_t begin = clock();

        /* only needs to be done once - saves to disk with better formatting for speed */
        /* if (format_repo("packages-meta-ext-v1.json", "aur-db.json"))
                return 1; */

        /*
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
        */
}
