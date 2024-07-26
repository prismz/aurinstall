#include "repo.h"
#include "string.h"
#include "options.h"

#include <alpm.h>
#include <string.h>
#include <time.h>
#include <json-c/json.h>

json_object *get_aur_pkg_meta(const char *name)
{
        json_object *meta;
        json_bool exists = json_object_object_get_ex(repo_data,
                        name, &meta);
        if (!exists)
                return NULL;
        return meta;
}
