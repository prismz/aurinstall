#ifndef INSTALL_H
#define INSTALL_H

#include "options.h"

#include <stdbool.h>
#include <json-c/json.h>

int download_package_source(const char *name, struct opts *opts);
bool build_files_exist(const char *name, struct opts *opts);
int install_packages(const char **targets, int n, struct opts *opts);

#endif  /* INSTALL_H */
