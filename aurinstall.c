/*
 * This file is part of aurinstall.
 *
 * aurinstall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * aurinstall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurinstall.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * Copyright (C) 2023 Hasan Zahra
 * https://github.com/prismz/aurinstall
 */

#include "json.h"
#include "alloc.h"
#include "requests.h"
#include "output.h"
#include "install.h"
#include "search.h"
#include "util.h"
#include "rpc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static char *user_cache_path;
#define VERSION "3.0"

void init(void)
{
        char *userhome = get_user_home();
        user_cache_path = safe_malloc(sizeof(char) * PATH_MAX);
        snprintf(user_cache_path, PATH_MAX, "%s/.cache/aurinstall", userhome);
       
        if (!dir_exists(user_cache_path)) {
                /* remove just in case the path exists but is not a folder. */
                snsystem("rm -rf %s", PATH_MAX + 32, user_cache_path);
                printf("Cache dir does not exist. Creating one at %s.\n", 
                                user_cache_path);
                snsystem("mkdir -p \"%s\"", PATH_MAX + 32, user_cache_path);
        }

        free(userhome);
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
        init();

        if (argc == 1) {
                usage();
        }

        /* parameters to pass to function */
        char *parameters[argc + 1];
        int parameters_i = 0;

        for (int i = 1; i < argc; i++) {
                if (argv[i][0] != '-') {
                        parameters[parameters_i++] = argv[i];
                }
        }

        char *operation = argv[1];
        
        if (strcmp(operation, "--search") == 0) {
                if (search_aur(parameters_i, parameters)) {

                }

                return 0;
        }
        
        if (strcmp(operation, "--update") == 0) {
                if (update_packages(user_cache_path)) {
                        fprintf(stderr, "failed to update packages.\n");
                        return 1;
                }

                return 0;

        } 

        if (strcmp(operation, "--remove") == 0) {
                if (remove_packages(parameters_i, parameters)) {
                        fprintf(stderr, "failed to remove packages.\n");
                        return 1;
                }

                return 0;
        } 

        if (strcmp(operation, "--clean") == 0) {
                if (clean_cache(user_cache_path)) {
                        fprintf(stderr, "failed to clean cache.\n");
                        return 1;
                }

                return 0;
        } 

        if  (strcmp(operation, "--help") == 0 
                        || strcmp(operation, "--usage") == 0) {
                usage();
                return 0;
        }

        if (strcmp(operation, "--version") == 0) {
                version();
                return 0;
        }

        if (operation[0] == '-') {  /* invalid operation */
                fprintf(stderr, "error: invalid operation.\n");
                return 1;
        }

        for (int i = 0; i < parameters_i; i++) {
                char *param = parameters[i];
                install_package(param, user_cache_path);
        }
}
