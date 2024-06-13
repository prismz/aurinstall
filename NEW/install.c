#include "install.h"
#include "aurinstall.h"
#include "rpc.h"
#include "requests.h"
#include "alloc.h"
#include "util.h"
#include "print.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <json-c/json.h>

struct hashmap *get_installed_pkgs(void)
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
                return installed_packages;
        }

        free(line_buf);
        pclose(fp);

        return installed_packages;
}

int get_installed_pkg_info(struct aurinstall_opts *opts)
{
        if (opts == NULL || opts->installed_packages == NULL)
                return 1;

        /* max length of request is 4443 bytes */
        char *request = safe_calloc(1, 4444);
        strcpy(request, "https://aur.archlinux.org/rpc/?v=5&type=info");
        size_t request_len = strlen(request);

        char *delim = "&arg[]=";
        size_t delim_len = strlen(delim);
        int done = 0;

        struct hashmap *installed = opts->installed_packages;

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

        opts->installed_packages_info = make_rpc_request(installed, request);

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

        int pad_amount = digits(uph->n);

        int *to_update_idx = safe_calloc(uph->n, sizeof(int));

        /* i tried memset, wouldn't work for some reason */
        for (size_t i = 0; i < uph->n; i++)
                to_update_idx[i] = 1;

        for (size_t i = 0; i < uph->n; i++) {
                struct package_info *pkg_info = uph->package_metadatas->infolist[i];
                if (pkg_info == NULL)
                        continue;

                char *name = pkg_info->name;
                char *installed_vers = pkg_info->inst_vers;
                char *ood = pkg_info->ood;
                char *vers = pkg_info->avail_vers;

                size_t name_padlen = namecol_len - strlen(name);

                char name_padding[namecol_len + 1];
                memset(name_padding, ' ', namecol_len);
                name_padding[namecol_len] = 0;

                char number_fmt[256];
                snprintf(number_fmt, 256, "%%0%dd", pad_amount);

                printf(number_fmt, i + 1);

                printf(". aur/%s%*.*s", name, (int)name_padlen,
                                (int)name_padlen, name_padding);

                print_version_difference(installed_vers, vers, verscol_len);

                printf("\n");
        }

        integer_exclude_prompt("packages to exclude? (1, 1-3, ^1) ",
                        (to_update_idx), uph->n);

        for (size_t i = 0; i < uph->n; i++)
                printf("%d\n", to_update_idx[i]);

        return 0;
}

int update(struct aurinstall_opts *opts)
{
        if (opts->installed_packages_info == NULL) {
                if (get_installed_pkg_info(opts) == NULL)
                        fatal_err("failed to get information on installed packages.");
        }

        int resultcount = opts->installed_packages_info->n;

        struct update_handle uph;
        uph.longest_pkg_name_len = 0;
        uph.longest_pkg_vers_len = 0;
        struct rpc_results *metadatas = safe_calloc(1, sizeof(struct rpc_results *));

        for (int i = 0; i < resultcount; i++) {
                struct package_info *info =
                        opts->installed_packages_info->infolist[i];
                if (info == NULL)
                        continue;

                if (strcmp(info->avail_vers, info->inst_vers) == 0)  /* equal versions */
                        continue;

                metadatas->infolist[metadatas->n++] = info;
        }

        if (metadatas->n == 0) {
                printf("no updates. exiting.\n");
                return 0;
        }

        uph.package_metadatas = metadatas;

        if (update_from_handle(opts, &uph))
                fatal_err("update failed.");

        free(uph.package_metadatas);

        return 0;
}

int install(struct aurinstall_opts *opts, const char *package_name)
{
}
