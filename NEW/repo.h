#ifndef REPO_H
#define REPO_H

#include <stdbool.h>
#include <json-c/json.h>
#include <alpm.h>

int download_repos(void);
int init_repo_data(void);
json_object *get_aur_pkg_meta(const char *name);

#endif  /* REPO_H */
