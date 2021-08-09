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
 * Copyright (C) 2021 Hasan Zahra
 * https://github.com/prismz/aurinstall
 */

#include "mem.h"
#include "util.h"
#include "requests.h"
#include "rpc.h"
#include "argparse.h"
#include "operations.h"

#include <stdio.h>
#include <stdlib.h>

void
usage(void)
{
   printf("Usage: aurinstall [OPTION]...\n\n");

   printf("  search               search the AUR using one or more searchterms.\n");
   printf("  install              install one or more packages from the AUR.\n");
   printf("  update               update currently installed AUR packages.\n");
   printf("  remove               remove one or more packages.\n");
   printf("  clean                clean the cache of downloaded packages.\n");
   printf("  help/usage           print this message and exit\n");
   printf("  version              print version and copyright info and exit\n\n");
   printf("https://github.com/prismz/aurinstall\n");
   exit(0);
}

void
version(void)
{
    printf("aurinstall v1.0\n");
    printf("Copyright (C) 2021 Hasan Zahra\n");
    printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    printf("This is free software: you are free to change and redistribute it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n\n");

    printf("Written by Hasan Zahra.\n");
    printf("https://github.com/prismz/aurinstall\n");
    exit(0);
}

int
main(int argc, char** argv)
{
    int arg_c;
    char** args = parse_args(argc, argv, &arg_c);
    if (arg_c < 1)
        usage();

    int oper = determine_operation(args[0]);
    if (oper == -1)
        die(stderr, "invalid operation.", 1);

    if (arg_c == 1 && oper != oper_update && oper != oper_clean && oper != oper_version && oper != oper_usage)
        die(stderr, "please provide an argument for the operation.", 1);

    char* home_folder = get_homedir();
    char* cache_path = smalloc(256, "cache_path - main() - aurinstall.c");
    snprintf(cache_path, 256, "%s/.cache/aurinstall", home_folder);
    sfree(home_folder);

    int done_updating = 0;

    switch (oper) {
        case oper_search:
            search_aur(args, arg_c);
            break;
        case oper_install:
            for (int i = 1; i < arg_c; i++)
                install_aur_package(args[i], cache_path);
            break;
        case oper_update:
            while (!done_updating)
                done_updating = !update_installed_packages(cache_path);
            break;
        case oper_clean:
            clean_package_cache(cache_path);
            break;
        case oper_remove:
            for (int i = 1; i < arg_c; i++)
                remove_package(args[i]); 
            break;
        case oper_version:
            version();
            break;
        case oper_usage:
            usage();
            break;
        default:
            printf("unimplemented.\n");
            break;
    }

    sfree(cache_path);
    for (int i = 0; i < arg_c; i++)
        sfree(args[i]);
    sfree(args);
}
