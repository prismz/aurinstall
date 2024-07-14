#ifndef DEPEND_H
#define DEPEND_H

#include "options.h"

#include <stdio.h>
#include <stdbool.h>
#include <alpm.h>

struct dep {
        char *repo;
        char *depstring;
        char *satisfier;
        bool is_aur;
        bool is_bdep;
        alpm_pkg_t *pkg;
        char *vers;
};

struct deplist {
        struct dep **dl;
        size_t cap;
        size_t n;
};

struct deplist *deplist_new(size_t cap);
struct dep *satisfy_dep(const char *depstring, struct opts *opts, bool is_bdep);
bool dep_satisfied(const char *depstring, struct opts *opts);
int get_package_dependencies(const char *package_name, struct deplist *dl,
                struct opts *opts);
struct deplist *install_dependencies(const char **targets, int n_targets, int *n,
                struct deplist *deps, struct opts *opts);

#endif  /* DEPEND_H */
