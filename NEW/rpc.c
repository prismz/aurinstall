#include "rpc.h"
#include "util.h"
#include "alloc.h"
#include "requests.h"

#include <stdio.h>
#include <stdlib.h>

struct rpc_results *make_rpc_request(struct hashmap *installed,
                const char *url)
{
        if (url == NULL)
                return NULL;

        char *response = requests_get(url);
        if (response == NULL)
                return NULL;

        struct rpc_results *res = safe_calloc(1, sizeof(struct rpc_results));
        res->request_result = json_tokener_parse(response);

        json_object *err;
        json_object_object_get_ex(res->request_result, "error", &err);
        if (err != NULL) {
                fatal_err("AUR RPC request failed with error %s",
                                json_object_get_string(err));
        }

        json_object *resultcount_json;
        json_object_object_get_ex(res->request_result, "resultcount",
                        &resultcount_json);
        res->n = json_object_get_int64(resultcount_json);

        json_object *results;
        json_object_object_get_ex(res->request_result, "results", &results);

        res->infolist = safe_calloc(res->n + 8,
                        sizeof(struct package_info *));
        res->capacity = res->n + 8;


        for (int i = 0; i < res->n; i++) {
                json_object *result = json_object_array_get_idx(results, i);

                json_object *vers_json, *name_json, *ood_json;
                json_object_object_get_ex(result, "Version", &vers_json);
                json_object_object_get_ex(result, "Name", &name_json);
                json_object_object_get_ex(result, "OutOfDate", &ood_json);

                const char *name = json_object_get_string(name_json);
                const char *vers = json_object_get_string(vers_json);
                const char *ood  = json_object_get_string(ood_json);

                char *installed_vers = (char *)hashmap_get(
                                installed,
                                name
                );

                struct package_info *pkg_info = safe_calloc(1,
                                sizeof(struct package_info));

                pkg_info->raw_json = result;
                pkg_info->name = safe_strdup(name);
                pkg_info->avail_vers = safe_strdup(vers);
                pkg_info->inst_vers = safe_strdup(installed_vers);
                pkg_info->ood = safe_strdup(ood);

                res->infolist[i] = pkg_info;
        }
}
