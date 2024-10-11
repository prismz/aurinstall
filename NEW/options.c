#include "options.h"
#include "util/repo.h"
#include "util/util.h"

#include <pacutils.h>
#include <alpm.h>

/* global variables */
char *cache_path;
char *repo_path;
char *root_program;  /* sudo/doas/other */
alpm_handle_t *alpm_handle;
alpm_list_t *sync_dbs;
alpm_db_t *localdb;
alpm_list_t *installed_packages;
json_object *repo_data;

static int read_opts_from_config(void)
{
        pu_config_t *config = pu_config_new();
        pu_ui_config_load(config, "/etc/pacman.conf");
        alpm_handle = pu_initialize_handle_from_config(config);
        if (!alpm_handle)
                return 1;
        sync_dbs = pu_register_syncdbs(alpm_handle, config->repos);
        if (!sync_dbs)
                return 1;
        return 0;
}

int init()
{
        cache_path    = "/home/anon/.cache/aurinstall/";
        repo_path     = "/home/anon/.cache/aurinstall/aur-repo.json";
        root_program  = "sudo";

        if (read_opts_from_config())
                fatal_err("libalpm error");
        if (init_repo_data())
                fatal_err("failed to initialize repos");

        localdb = alpm_get_localdb(alpm_handle);
        sync_dbs = alpm_get_syncdbs(alpm_handle);
        installed_packages = alpm_db_get_pkgcache(localdb);

        return 0;
}
