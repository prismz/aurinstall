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

#endif
