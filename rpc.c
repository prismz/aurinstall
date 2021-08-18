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

#include "rpc.h"
#include "util.h"
#include "mem.h"
#include "requests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <json-c/json.h>

struct api_results*
parse_api_results(struct curl_str* cs)
{
    struct api_results* ar = smalloc(sizeof(struct api_results));

    struct json_object* jobj_results = json_tokener_parse(cs->ptr);
    struct json_object* jobj_type;
    struct json_object* jobj_resultcount;

    /* handle error responses */
    json_object_object_get_ex(jobj_results, "type", &jobj_type);
    const char* type = json_object_get_string(jobj_type);

    if (type != NULL) {
        ar->type = smalloc(sizeof(char) * (strlen(type) + 1));
        strcpy(ar->type, type);
    }

    if (!strcmp(type, "error")) {
        struct json_object* jobj_err;
        json_object_object_get_ex(jobj_results, "error", &jobj_err);
        const char* err = json_object_get_string(jobj_err);

        if (err != NULL) {
            ar->error = smalloc(sizeof(char) * (strlen(err) + 1));
            strcpy(ar->error, err);
        }
        // json_object_put(jobj_err);
    }

    json_object_object_get_ex(jobj_results, "resultcount", &jobj_resultcount);
    json_object_object_get_ex(jobj_results, "results", &ar->results);

    ar->resultcount = json_object_get_int(jobj_resultcount);

    // json_object_put(jobj_results);
    return ar;
}

struct package_data*
parse_package_data(struct json_object* result)
{
    struct package_data* pi = smalloc(sizeof(struct package_data));

    struct json_object* jobj_name;
    struct json_object* jobj_desc;
    struct json_object* jobj_ver;
    struct json_object* jobj_ood;

    json_object_object_get_ex(result, "Name", &jobj_name);
    json_object_object_get_ex(result, "Description", &jobj_desc);
    json_object_object_get_ex(result, "Version", &jobj_ver);
    json_object_object_get_ex(result, "OutOfDate", &jobj_ood);

    const char* name = json_object_get_string(jobj_name);
    const char* desc = json_object_get_string(jobj_desc);
    const char* ver = json_object_get_string(jobj_ver);
    const char* ood = json_object_get_string(jobj_ood);

    if (name != NULL) {
        pi->name = smalloc(sizeof(char) * (strlen(name) + 1));
        strcpy(pi->name, name);
    } else {
        pi->name = NULL;
    }

    if (desc != NULL) {
        pi->desc = smalloc(sizeof(char) * (strlen(desc) + 1));
        strcpy(pi->desc, desc);
    } else {
        pi->desc = NULL;
    }

    if (ver != NULL) {
        pi->ver = smalloc(sizeof(char) * (strlen(ver) + 1));
        strcpy(pi->ver, ver);
    } else {
        pi->ver = NULL;
    }

    if (ood != NULL) {
        pi->ood = smalloc(sizeof(char) * (strlen(ood) + 1));
        strcpy(pi->ood, ood);
    } else {
        pi->ood = NULL;
    }

    return pi;
}

void
free_package_data(struct package_data* pi)
{
    if (pi->name != NULL)
        sfree(pi->name);
    if (pi->desc != NULL)
        sfree(pi->desc);
    if (pi->ver != NULL)
        sfree(pi->ver);
    if (pi->ood != NULL)
        sfree(pi->ood);
    sfree(pi);
}

void
free_api_results(struct api_results* ar)
{
    sfree((char*)ar->type);
    sfree((char*)ar->error);
    // json_object_put(ar->results);
    sfree(ar);
}

int
pkg_array_contains(struct package_data** haystack, size_t n, char* needle)
{
    for (int i = 0; (size_t)i < n; i++) {
        if (!strcmp(haystack[i]->name, needle))
            return 1;
    }
    return 0;
}