#include "install.h"
#include "aurinstall.h"
#include "requests.h"
#include "alloc.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <json-c/json.h>

int get_installed_pkgs(struct aurinstall_opts *opts)
{
        struct hashmap *installed_packages = hashmap_new(free);

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

                char *version = safe_strdup(version_ro);

                if (hashmap_insert(installed_packages, name_ro, version) < 0)
                        fatal_err("failed to set hashmap item.");
        }

        if (installed_packages->n_buckets == 0) {
                hashmap_free(installed_packages);
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
        struct hashmap *installed = opts->installed_packages;
        if (installed == NULL)
                return 1;

        /* max length of request is 4443 bytes */
        char *request = safe_calloc(1, 4444);
        strcpy(request, "https://aur.archlinux.org/rpc/?v=5&type=info");
        size_t request_len = strlen(request);

        char *delim = "&arg[]=";
        size_t delim_len = strlen(delim);
        int done = 0;

        for (size_t i = 0; i < installed->capacity; i++) {
                struct bucket *atom = installed->buckets[i];
                struct bucket *curr;
                if (atom == NULL)
                        continue;

                while (atom != NULL) {
                        curr = atom;
                        atom = atom->next;

                        size_t name_len = strlen(curr->key);

                        if (request_len + delim_len + name_len > 4443) {
                                warning("too many installed packages to get info in one request. if you are updating, simply update again.");
                                done = 1;
                                break;
                        }

                        request_len += delim_len + strlen(curr->key);

                        strcat(request, delim);
                        strcat(request, curr->key);
                }

                if (done)
                        break;
        }

        char *response = requests_get(request);
        if (response == NULL)
                fatal_err("failed to query AUR rpc.");

        json_object *json = json_tokener_parse(response);

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

/* assumes struct is valid */
static int update_from_handle(struct aurinstall_opts *opts,
                struct update_handle *uph)
{
        if (uph == NULL || uph->package_metadatas == NULL)
                return 1;

        size_t namecol_len = uph->longest_pkg_name_len + 2;
        size_t verscol_len = uph->longest_pkg_vers_len + 2;

        for (size_t i = 0; i < uph->n; i++) {
                json_object *result = uph->package_metadatas[i];
                json_object *vers_json, *name_json, *ood_json;
                json_object_object_get_ex(result, "Version", &vers_json);
                json_object_object_get_ex(result, "Name", &name_json);
                json_object_object_get_ex(result, "OutOfDate", &ood_json);

                const char *name = json_object_get_string(name_json);
                const char *vers = json_object_get_string(vers_json);
                const char *ood  = json_object_get_string(ood_json);

                char *installed_vers = (char *)hashmap_get(
                                opts->installed_packages,
                                name
                );
                size_t name_padlen = namecol_len - strlen(name);
                size_t vers_padlen = verscol_len - strlen(installed_vers);

                char name_padding[namecol_len + 1];
                memset(name_padding, ' ', namecol_len);
                name_padding[namecol_len] = 0;

                char vers_padding[verscol_len + 1];
                memset(vers_padding, ' ', verscol_len);
                vers_padding[verscol_len] = 0;

                printf("aur/%s%*.*s", name, (int)name_padlen,
                                (int)name_padlen, name_padding);
                printf("%s%*.*s", installed_vers, (int)vers_padlen,
                                (int)vers_padlen, vers_padding);
                printf(" ->  ");
                printf("%s", vers);

                printf("\n");
        }

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

        json_object *resultlist;
        json_object_object_get_ex(j, "results", &resultlist);

        /* TODO: handle this better */
        if (resultcount == 0)
                return 0;

        struct update_handle uph;
        uph.n = 0;
        uph.longest_pkg_name_len = 0;
        uph.longest_pkg_vers_len = 0;
        uph.capacity = (size_t)((float)resultcount /
                        (float)(HM_HASHMAP_MAX_LOAD));
        uph.package_metadatas = safe_calloc(uph.capacity,
                        sizeof(json_object *));

        for (int i = 0; i < resultcount; i++) {
                json_object *result = json_object_array_get_idx(resultlist, i);

                json_object *vers_json, *name_json, *ood_json;
                json_object_object_get_ex(result, "Version", &vers_json);
                json_object_object_get_ex(result, "Name", &name_json);
                json_object_object_get_ex(result, "OutOfDate", &ood_json);

                const char *vers = json_object_get_string(vers_json);
                const char *name = json_object_get_string(name_json);
                const char *ood  = json_object_get_string(ood_json);

                char *installed_vers = (char *)hashmap_get(
                                opts->installed_packages,
                                name
                );

                if (ood != NULL)
                        warning("installed package %s is out of date.", name);

                if (strcmp(vers, installed_vers) == 0)  /* equal versions */
                        continue;

                size_t namelen = strlen(name);
                size_t verslen = strlen(installed_vers);

                if (namelen > uph.longest_pkg_name_len)
                        uph.longest_pkg_name_len = namelen;
                if (verslen > uph.longest_pkg_vers_len)
                        uph.longest_pkg_vers_len = verslen;

                uph.package_metadatas[uph.n++] = result;
        }

        if (uph.n == 0)
                printf("no updates. exiting.\n");

        if (update_from_handle(opts, &uph))
                fatal_err("update failed.");

        free(uph.package_metadatas);

        return 0;
}

int install(struct aurinstall_opts *opts, const char *package_name)
{
}
