#ifndef OPTIONS_H
#define OPTIONS_H

#include <json-c/json.h>
#include <alpm.h>

struct opts {
        char *cache_path;
        char *packages_path;
        char *metadata_path;
        char *repo_path;
        json_object *repo_data;

        /* ALPM */
        alpm_handle_t *alpm_handle;
        alpm_list_t *sync_dbs;
        alpm_db_t *localdb;
};

int read_opts_from_config(struct opts *opts);
int parse_pacman_opts(struct opts *opts);

#endif  /* OPTIONS_H */
