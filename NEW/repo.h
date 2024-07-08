#ifndef REPO_H
#define REPO_H

#include "options.h"

#include <stdbool.h>
#include <json-c/json.h>
#include <alpm.h>

int download_repos(struct opts *opts);
int init_repo_data(struct opts *opts);
json_object *get_aur_pkg_meta(const char *name, struct opts *opts);

#endif  /* REPO_H */
