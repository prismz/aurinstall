#include "argparse.h"
#include "util.h"
#include "mem.h"
#include "operations.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

/* 
 * nargc is a pointer to integer of the 
 * amount of arguments in the returned list 
 */
char**
parse_args(int argc, char** argv, int* nargc_l)
{
    char** nargv = smalloc(sizeof(char*) * (argc - 1), "nargv - parse_args() - argparse.c");
    int nargc = 0;
    for (int i = 1; i < argc; i++) {
        char* carg = argv[i];
        if (carg[0] != '-') {
            nargv[nargc] = smalloc(strlen(carg) + 1, "nargv[nargc] - parse_args() - argparse.c");
            strcpy(nargv[nargc], argv[i]);
            nargc++;
        }
    }
    *nargc_l = nargc;
    return nargv;
}

int
determine_operation(char* arg)
{
    if (!strcmp(arg, "install"))
        return oper_install;
    else if (!strcmp(arg, "search"))
        return oper_search;
    else if (!strcmp(arg, "update"))
        return oper_update;
    else if (!strcmp(arg, "clean"))
        return oper_clean;
    else if (!strcmp(arg, "remove"))
        return oper_remove;
    return -1;
}
