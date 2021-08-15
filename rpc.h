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

#ifndef RPC_H
#define RPC_H

#include "requests.h"

#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>

struct api_results {
    int resultcount;
    struct json_object* results;
    char* type;
    char* error;
};

struct package_data {
    char* name;
    char* desc;
    char* ver;
    char* ood;
};

struct api_results* parse_api_results(struct curl_str* cs);
struct package_data* parse_package_data(struct json_object* result);
void free_package_data(struct package_data* pi);
void free_api_results(struct api_results* ar);
int pkg_array_contains(struct package_data** haystack, size_t n, char* needle);

#endif