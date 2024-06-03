#ifndef INSTALL_H
#define INSTALL_H

#include "aurinstall.h"

int get_installed_pkgs(struct aurinstall_opts *opts);
int get_installed_pkg_info(struct aurinstall_opts *opts);
int update(struct aurinstall_opts *opts);

#endif  /* INSTALL_H */
