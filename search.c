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

static char *levenshtein_cmp_str;

/* https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C */
int levenshtein(char *s1, char *s2) {
        unsigned int s1len, s2len, x, y, lastdiag, olddiag;
        s1len = strlen(s1);
        s2len = strlen(s2);
        unsigned int column[s1len + 1];
        for (y = 1; y <= s1len; y++)
                column[y] = y;
        for (x = 1; x <= s2len; x++) {
                column[0] = x;
                for (y = 1, lastdiag = x - 1; y <= s1len; y++) {
                        olddiag = column[y];
                        column[y] = MIN3(column[y] + 1, column[y - 1] + 1, 
                                lastdiag + (s1[y-1] == s2[x - 1] ? 0 : 1));
                        lastdiag = olddiag;
                }
        }

        return column[s1len];
}

int package_qsort_levenshtein(const void *one, const void *two)
{
        struct json *pkg_one = (struct json *)*(const struct json **)one;
        struct json *pkg_two = (struct json *)*(const struct json **)two;

        char *str = levenshtein_cmp_str;

        char *one_str = json_get_dict_string(pkg_one, "Name"); 
        char *two_str = json_get_dict_string(pkg_two, "Name"); 

        int diff = levenshtein(one_str, str) - levenshtein(two_str, str);

        return diff;
}

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

        /* 
         * We sort the results using the Levenshtein sorting algorithm.
         * Create an array to store all results to sort later.
         */
        int packages_i = 0;
        struct json **packages = safe_calloc(data->resultcount, 
                        sizeof(struct json));

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

                if (valid)
                        packages[packages_i++] = pkg;
        }

        levenshtein_cmp_str = terms[0];
        qsort(packages, packages_i, sizeof(struct json *), 
                        package_qsort_levenshtein);

        for (int i = packages_i - 1; i >= 0; i--) {
                struct json *pkg = packages[i];

                char *name = json_get_dict_string(pkg, "Name");
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

        free(packages);
        free_rpc_data(data);
        free_hashmap(installed);

        return 0;
}
