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
    struct api_results* ar = smalloc(sizeof(struct api_results), "ar - parse_api_results() - rpc.c");

    struct json_object* jobj_results = json_tokener_parse(cs->ptr);
    struct json_object* jobj_type;
    struct json_object* jobj_resultcount;

    /* handle error responses */
    json_object_object_get_ex(jobj_results, "type", &jobj_type);
    const char* type = json_object_get_string(jobj_type);

    if (type != NULL) {
        ar->type = smalloc(sizeof(char) * (strlen(type) + 1), "ar->type - parse_api_results() - rpc.c");
        strcpy(ar->type, type);
    }

    if (!strcmp(type, "error")) {
        struct json_object* jobj_err;
        json_object_object_get_ex(jobj_results, "error", &jobj_err);
        const char* err = json_object_get_string(jobj_err);

        if (err != NULL) {
            ar->error = smalloc(sizeof(char) * (strlen(err) + 1), "ar->error - parse_api_results() - rpc.c");
            strcpy(ar->error, err);
        }
    }

    json_object_object_get_ex(jobj_results, "resultcount", &jobj_resultcount);
    json_object_object_get_ex(jobj_results, "results", &ar->results);

    ar->resultcount = json_object_get_int(jobj_resultcount);

    return ar;
}

struct package_data*
parse_package_data(struct json_object* result)
{
    struct package_data* pi = smalloc(sizeof(struct package_data), "pi - parse_package_data() - rpc.c");

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
        pi->name = smalloc(sizeof(char) * (strlen(name) + 5), "pi->name - parse_package_data() - rpc.c");
        strcpy(pi->name, name);
    } else {
        pi->name = NULL;
    }

    if (desc != NULL) {
        pi->desc = smalloc(sizeof(char) * (strlen(desc) + 5), "pi->desc - parse_package_data() - rpc.c");
        strcpy(pi->desc, desc);
    } else {
        pi->desc = NULL;
    }

    if (ver != NULL) {
        pi->ver = smalloc(sizeof(char) * (strlen(ver) + 5), "pi->ver - parse_package_data() - rpc.c");
        strcpy(pi->ver, ver);
    } else {
        pi->ver = NULL;
    }

    if (ood != NULL) {
        pi->ood = smalloc(sizeof(char) * (strlen(ood) + 5), "pi->ood - parse_package_data() - rpc.c");
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
}

void
free_api_results(struct api_results* ar)
{
    sfree((char*)ar->type);
    sfree((char*)ar->error);
}