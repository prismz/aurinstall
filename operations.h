#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdio.h>
#include <stdlib.h>

enum operation {
    oper_install,
    oper_search,
    oper_update,
    oper_clean,
    oper_remove
};

void search_aur(char** searchterms, size_t searchterm_count);
void install_aur_package(char* name, char* cache_dir);
int update_installed_packages(char* cache_dir);
void clean_package_cache(char* cache_dir);
void remove_package(char* name);


#endif
