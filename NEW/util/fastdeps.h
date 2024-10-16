#ifndef FASTDEPS_H
#define FASTDEPS_H

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <json-c/json.h>

#define time_since(begin) ((double)(clock() - begin) / CLOCKS_PER_SEC)

typedef struct fastdeps_target Target;
typedef struct fastdeps_targetlist TargetList;

/* non-AUR packages will only have a name - pacman should handle the rest */
typedef struct fastdeps_target {
        char *name;
        char *desc;
        char *version;

        bool aur;

        TargetList *depends;
        TargetList *makedepends;
} Target;

typedef struct fastdeps_targetlist{
        Target **targets;
        size_t capacity;
        size_t n;
} TargetList;

Target *target_new(const char *name);
TargetList *targetlist_new(size_t capacity);
void target_free(Target *t);
void targetlist_free(TargetList *tl);
int targetlist_append(TargetList *tl, Target *t);
TargetList *parse_deplist(json_object *deps);
json_object *package_get_metadata(const char *name);
TargetList *get_target_list(const char **target_names, int n);
void _print_targetlist(TargetList *tl, int depth, bool are_makedepends);
#define print_targetlist(tl) (_print_targetlist(tl, 0, false))

#endif  /* FASTDEPS_H */
