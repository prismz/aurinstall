#ifndef INSTALL_H
#define INSTALL_H

#include "aurinstall.h"

struct update_handle {
        json_object **package_metadatas;
        size_t n;
        size_t capacity;

        /* for formatting */
        size_t longest_pkg_name_len;
        size_t longest_pkg_vers_len;
};

int get_installed_pkgs(struct aurinstall_opts *opts);
int get_installed_pkg_info(struct aurinstall_opts *opts);
int update(struct aurinstall_opts *opts);

#endif  /* INSTALL_H */
