#ifndef MAKEPKG_H
#define MAKEPKG_H

struct srcinfo {
        char *name;
        char *group;

        char **dependencies;
        int n_dependencies;

        char **pgp_keys;
        int n_pgp_keys;
};

#endif  /* MAKEPKG_H */
