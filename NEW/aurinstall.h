#ifndef AURINSTALL_H
#define AURINSTALL_H

#include "hashmap.h"

#include <stdbool.h>
#include <json-c/json.h>

#define DEFAULT_CONFIG_PATH "/etc/aurinstall-config.json"
//#define DEFAULT_CONFIG_PATH "/home/hasan/Development/aurinstall/NEW/default-config.json"
#define VERSION "4.0"

struct aurinstall_opts {
        char *cache_path;
        /* map installed packages and their versions */
        HashMap *installed_packages;
        json_object *installed_packages_info;
        bool color;
};

#endif  /* AURINSTALL_H */
