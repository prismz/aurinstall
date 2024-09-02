#ifndef TARGET_H
#define TARGET_H

#include <stdio.h>
#include <stdbool.h>

struct target_list;
struct target;

struct target {
        char *name;
        char *ver;

        bool is_installed;
        bool is_explicit;
        bool is_makedep;
        bool in_repos; /* if false, AUR */

        /* should be NULL if not an AUR package - pacman should handle it */
        struct target_list *dependencies;
};

struct target_list {
        struct target **targets;
        size_t capacity;
        int n;
};

struct target *target_new(const char *name, const char *ver, bool is_installed,
                bool is_explicit, bool is_makedep, bool in_repos,
                struct target_list *dependencies);
struct target_list *target_list_new(size_t capacity);
void target_list_free(struct target_list *tl);
void target_free(struct target *t);
int target_list_append(struct target_list *tl, struct target *t);
struct target *get_target_from_name(const char *target_name, bool is_explicit, bool is_makedep);
struct target_list *get_target_list(char **explicit_targets, int n);
void target_list_print(struct target_list *tl, int depth);

#endif  /* TARGET_H */
