#include "install.h"
#include "aurinstall.h"
#include "requests.h"
#include "alloc.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

int get_installed_pkgs(struct aurinstall_opts *opts)
{
        HashMap *installed_packages = new_hashmap(16);

        FILE *fp = popen("pacman -Qm --color=never", "r");
        char *line_buf = safe_malloc(sizeof(char) * 1024);

        while (fgets(line_buf, 1024, fp) != NULL) {
                size_t len = strlen(line_buf);
                line_buf[len - 1] = '\0';
                len--;

                int splitspace_idx = -1;
                for (size_t i = 0; i < len; i++) {
                        if (line_buf[i] == ' ') {
                                splitspace_idx = (int)i;
                                break;
                        }
                }

                if (splitspace_idx == -1)
                        fatal_err("invalid package atom: %s", line_buf);

                line_buf[splitspace_idx] = '\0';
                char *version_ro = line_buf + splitspace_idx + 1;
                char *name_ro = line_buf;

                char *name = safe_strdup(name_ro);
                char *version = safe_strdup(version_ro);

                HMItem *item = new_item(name, version, free, free);
                if (hashmap_set(installed_packages, item) != 0)
                        fatal_err("failed to set hashmap item.");
        }

        if (installed_packages->stored == 0) {
                free_hashmap(installed_packages);
                opts->installed_packages = NULL;
                return 0;
        }

        free(line_buf);
        pclose(fp);

        opts->installed_packages = installed_packages;
        return 0;
}

int get_installed_pkg_info(struct aurinstall_opts *opts)
{
        HashMap *installed = opts->installed_packages;
        if (installed == NULL)
                return 1;

        /* max length of request is 4443 bytes */
        char *request = safe_calloc(1, 4444);
        strcpy(request, "https://aur.archlinux.org/rpc/?v=5&type=info");
        size_t request_len = strlen(request);

        for (size_t i = 0; i < installed->can_store; i++) {
                HMItem *atom = installed->items[i];
                if (atom == NULL)
                        continue;

                char *delim = "&arg[]=";
                size_t delim_len = strlen(delim);
                size_t name_len = strlen(atom->key);

                if (request_len + delim_len + name_len > 4443) {
                        err("too many installed packages to get info in one request. if you are updating, simply update again.");
                        break;
                }
                request_len += delim_len + strlen(atom->key);

                strcat(request, delim);
                strcat(request, atom->key);
        }

        char *response = requests_get(request);
        if (response == NULL)
                fatal_err("failed to query AUR rpc.");

        json_object *json = json_tokener_parse(response);

        printf("RESPONSE: %s\n", response);

        free_hashmap(installed);
        free(request);
        free(response);

        json_object *err;
        json_object_object_get_ex(json, "error", &err);
        if (err != NULL) {
                fatal_err("AUR request failed with error %s",
                                json_object_get_string(err));
        }

        opts->installed_packages_info = json;

        return 0;
}

int update(struct aurinstall_opts *opts)
{
        if (opts->installed_packages_info == NULL) {
                if (get_installed_pkg_info(opts))
                        fatal_err("failed to get information on installed packages.");
        }

        json_object *j = opts->installed_packages_info;
        json_object *resultcount_json;
        json_object_object_get_ex(j, "resultcount", &resultcount_json);
        int resultcount = json_object_get_int64(resultcount_json);
        printf("%d request results.\n", resultcount);

        return 0;
}
