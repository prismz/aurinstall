#include "aurinstall.h"
#include "install.c"
#include "alloc.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>

int parse_config(struct aurinstall_opts *opts)
{
        char *config_contents = read_file(DEFAULT_CONFIG_PATH);
        if (config_contents == NULL)
                fatal_err("failed to read config file.");

        json_object *cfg_json = json_tokener_parse(config_contents);
        
        /* cache dir */
        json_object *cachedir_path_json;
        json_object_object_get_ex(cfg_json, "cache_path", &cachedir_path_json);
        opts->cache_path = safe_strdup(
                        json_object_get_string(cachedir_path_json)
        );
       
        printf("CACHE PATH: %s\n", opts->cache_path);
        return 0;
}

void init(struct aurinstall_opts *opts)
{
        /* parse config file, generating
         * one and prompting user if it doesn't exist */
        if (file_exists(DEFAULT_CONFIG_PATH)) {
                if (parse_config(opts))
                        fatal_err("couldn't parse config at %s.", 
                                        DEFAULT_CONFIG_PATH);
        } else {
                fatal_err("config file doesn't exist. reinstall to fix.");
        }

        if (!dir_exists(opts->cache_path))
                fatal_err("cache directory doesn't exist. reinstall to fix.");

        /* get list of installed packages and check for missing dependencies
         * and installed out of date packages. */
        if (validate_installed())
                fatal_err("failed to validate installed packages.");

}

void usage(void)
{
        printf("Usage: aurinstall [PACKAGE1] [PACKAGE2] [OPTION]...\n\n");

        printf("  --search               search the AUR using one or more searchterms.\n");
        printf("  --update               update currently installed AUR packages.\n");
        printf("  --remove               remove one or more packages.\n");
        printf("  --clean                clean the cache of downloaded packages.\n");
        printf("  --help/usage           print this message and exit.\n");
        printf("  --version              print version and copyright info and exit.\n\n");

        printf("https://github.com/prismz/aurinstall\n");

        exit(0);
}

void version(void)
{
        printf("aurinstall %s\n", VERSION);

        printf("Copyright (C) 2023 Hasan Zahra\n");
        printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
        printf("This is free software: you are free to change and redistribute it.\n");
        printf("There is NO WARRANTY, to the extent permitted by law.\n\n");

        printf("Written by Hasan Zahra.\n");
        printf("https://github.com/prismz/aurinstall\n");

        exit(0);
}

int main(int argc, char **argv)
{
        struct aurinstall_opts opts;
        init(&opts);

        free(opts.cache_path);
}
