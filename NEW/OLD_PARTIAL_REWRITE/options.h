#ifndef OPTIONS_H
#define OPTIONS_H

#include <json-c/json.h>
#include <alpm.h>

extern char *cache_path;
extern char *packages_path;
extern char *metadata_path;
extern char *repo_path;
extern char *root_program;  /* sudo/doas/other */
extern alpm_handle_t *alpm_handle;
extern alpm_list_t *sync_dbs;
extern alpm_db_t *localdb;
extern alpm_list_t *installed_packages;
extern json_object *repo_data;

int init(void);

#endif  /* OPTIONS_H */
