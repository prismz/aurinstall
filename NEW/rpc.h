#ifndef RPC_H
#define RPC_H

#include "hashmap.h"

#include <stdio.h>
#include <stdbool.h>
#include <json-c/json.h>

struct package_info {
        json_object *raw_json;
        char *name;
        char *avail_vers;
        char *inst_vers;
        char *ood;
};

struct rpc_results {
        json_object *request_result;
        struct package_info **infolist;
        size_t capacity;
        int n;

        /* for formatting */
        size_t longest_installed_vers_len;
        size_t longest_name_len;
};

struct rpc_results *make_rpc_request(struct hashmap *installed,
                const char *url);

#endif  /* RPC_H */
