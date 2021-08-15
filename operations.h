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

#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdio.h>
#include <stdlib.h>

enum operation {
    oper_install,
    oper_search,
    oper_update,
    oper_clean,
    oper_remove,
    oper_version,
    oper_usage
};

struct package_data**
get_installed_packages(int* npkg, int* max_name_len,
                       int* max_ver_len, char* information_request,
                       size_t* request_len, int* call_again);
                       
void search_aur(char** searchterms, size_t searchterm_count);
void install_aur_package(char* name, char* cache_dir);
int update_installed_packages(char* cache_dir);
void clean_package_cache(char* cache_dir);
void remove_packages(char* packages);

#endif