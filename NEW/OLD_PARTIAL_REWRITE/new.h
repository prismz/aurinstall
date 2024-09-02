#ifndef NEW_H
#define NEW_H

#include <stdio.h>
#include <stdbool.h>

struct target_list;
struct target;

struct target {
        char *name;
        char *ver;

        bool is_installed;
        bool is_explicit;
        bool is_bdep;
        bool is_repodep; /* if false, AUR */

        struct target_list *dependencies;
};

struct target_list {
        struct target **targets;
        size_t size;
        int n;
};

int n_install_packages(char **targets, int n);

#endif  /* NEW_H */
