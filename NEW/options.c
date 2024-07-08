#include "options.h"
#include "alloc.h"
#include "string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pacutils.h>
#include <alpm.h>

int read_opts_from_config(struct opts *opts)
{
        pu_config_t *config = pu_config_new();
        pu_ui_config_load(config, "/etc/pacman.conf");
        opts->alpm_handle = pu_initialize_handle_from_config(config);
        if (!opts->alpm_handle)
                return 1;
        opts->sync_dbs = pu_register_syncdbs(opts->alpm_handle, config->repos);
        if (!opts->sync_dbs)
                return 1;
        return 0;
}
