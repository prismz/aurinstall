#include "aurinstall.h"
#include "depend.h"
#include "repo.h"
#include "install.h"
#include "util.h"
#include "depend.h"
#include "options.h"

#include <alpm.h>
#include <stdio.h>
#include <stdbool.h>

int init(struct opts *opts)
{
        opts->cache_path    = "/home/anon/.cache/aurinstall/";
        opts->packages_path = "/home/anon/.cache/aurinstall/packages";
        opts->metadata_path = "/home/anon/.cache/aurinstall/metadata.json";
        opts->repo_path     = "/home/anon/.cache/aurinstall/aur-repo.json";

        if (init_repo_data(opts))
                fatal_err("failed to initialize repos...");

        if (read_opts_from_config(opts))
                fatal_err("libalpm error");

        opts->localdb = alpm_get_localdb(opts->alpm_handle);
        opts->sync_dbs = alpm_get_syncdbs(opts->alpm_handle);

        return 0;
}

int main(void)
{
        struct opts opts;
        init(&opts);

        const char *targets[] = { "ungoogled-chromium-bin", NULL };
        struct deplist *dl = get_targets_dependencies(targets, 1, &opts);
        dependency_prompt(dl, &opts);

}
