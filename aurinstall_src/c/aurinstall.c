#include "mem.h"
#include "util.h"
#include "requests.h"
#include "rpc.h"
#include "argparse.h"
#include "operations.h"

#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char** argv)
{
    int arg_c;
    char** args = parse_args(argc, argv, &arg_c);
    if (arg_c < 1)
        die(stderr, "no operation specified.", 1);

    int oper = determine_operation(args[0]);
    if (oper == -1)
        die(stderr, "invalid operation.", 1);

    if (arg_c == 1 && oper != oper_update && oper != oper_clean)
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
        default:
            printf("unimplemented.\n");
            break;
    }

    sfree(cache_path);
    for (int i = 0; i < arg_c; i++)
        sfree(args[i]);
    sfree(args);
}
