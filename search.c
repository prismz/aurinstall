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

#include "search.h"
#include "requests.h"
#include "install.h"
#include "output.h"
#include "alloc.h"
#include "json.h"
#include "rpc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void print_search_result(bool istty, char *name, char *desc, 
                char *ver, int ood, bool installed)
{
        if (!istty) {
                printf("aur/%s %s", name, ver);
                if (ood)
                        printf(" (OUT OF DATE)");

                if (installed)
                        printf(" [INSTALLED]");
                
                printf("\n");
                if (desc != NULL)
                        printf("    %s\n", desc);

                return;
        }

        printf("%s%saur/%s%s%s %s%s%s%s", BOLD, RED, ENDC, name, ENDC, ENDC, 
                        GREEN, ver, ENDC);
        if (ood)
                printf(" %s(OUT OF DATE)%s", RED, ENDC);
        if (installed)
                printf(" %s[INSTALLED]%s", BLUE, ENDC);

        printf("\n");
        if (desc != NULL)
                indent_print(desc, 4);

}

int search_aur(int n, char **terms)
{
        if (n <= 0)
                return 1;

        HashMap *installed = get_installed_packages();
        if (installed == NULL) {
                return 1;        
        }

        char *url = safe_malloc(1024);
        snprintf(url, 1024, 
                        "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s",
                        terms[0]);

        struct rpc_data *data = make_rpc_request(url);
        
        free(url);

        if (data == NULL) {
                return 1;
        }

       
        if (data->type != rpc_search) {
                return 1;
        }

        bool istty = stdout_is_tty();

        for (size_t i = 0; i < data->resultcount; i++) {
                struct json *pkg = json_get_array_item(data->results, i);
                char *name = json_get_dict_string(pkg, "Name");
                bool valid = true;
                for (int j = 1; j < n; j++) {
                        if (strstr(name, terms[j]) == NULL) {
                                valid = false;
                                break;
                        }
                }

                if (valid) {
                        char *desc = json_get_dict_string(pkg, "Description");
                        char *ver = json_get_dict_string(pkg, "Version");
                        int ood = json_get_dict_number(pkg, "OutOfDate");
                        
                        bool is_installed = false;
                        if (hashmap_index(installed, name) != NULL) {
                                is_installed = true;
                        }
                        
                        print_search_result(istty, name, desc, ver, ood,
                                        is_installed);
                }
        }

        free_rpc_data(data);

        return 0;
}
