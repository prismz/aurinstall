#ifndef INSTALL_H
#define INSTALL_H

#include "options.h"

#include <stdbool.h>
#include <json-c/json.h>

int download_package_source(const char *name);
bool build_files_exist(const char *name);
int install_packages(const char **targets, int n);

#endif  /* INSTALL_H */
