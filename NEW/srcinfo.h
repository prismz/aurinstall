#ifndef SRCINFO_H
#define SRCINFO_H

#include <stdio.h>

struct pgp_key {
        char *key;

        /* package that provides this pgp key */
        char *from;
};

struct pgp_keylist {
        struct pgp_key **keys;
        size_t capacity;
        size_t n;
};

struct srcinfo {
        char *pkgbase;
        char *pkgname;
        char *pkgver;

        char **keys;
        size_t keys_cap;
        size_t n_keys;

        struct deplist *deps;
};

struct pgp_keylist *get_pgp_keys(const char *pkg_name);

#endif  /* SRCINFO_H */
