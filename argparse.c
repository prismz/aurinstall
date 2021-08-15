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
    char** nargv = smalloc(sizeof(char*) * (argc - 1));
    int nargc = 0;
    for (int i = 1; i < argc; i++) {
        char* carg = argv[i];
        if (carg[0] != '-') {
            nargv[nargc] = smalloc(sizeof(char) * (strlen(carg) + 1));
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
    else if (!strcmp(arg, "usage") || !strcmp(arg, "help"))
        return oper_usage;
    else if (!strcmp(arg, "version"))
        return oper_version;

    return -1;
}