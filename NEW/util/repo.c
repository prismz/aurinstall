#include "repo.h"
#include "util.h"
#include "alloc.h"
#include "string.h"
#include "requests.h"
#include "../options.h"

#include <alpm.h>
#include <string.h>
#include <time.h>
#include <json-c/json.h>

static json_object *get_formatted_repo_data(void)
{
        char *pkg_list_path = packages_path;
        char *pkg_meta_path = metadata_path;

        FILE *list_fp = fopen(pkg_list_path, "rb");
        json_object *metadata = json_object_from_file(pkg_meta_path);

        if (!list_fp)
                return NULL;

        json_object *formatted_repodata = json_object_new_object();

        int resultcount = json_object_array_length(metadata);
        json_object_object_add(formatted_repodata, "resultcount",
                        json_object_new_int64(resultcount));

        time_t current_time = time(NULL);
        json_object_object_add(formatted_repodata, "lastsync",
                        json_object_new_int64(current_time));

        struct tm lt;
        char human_time[128];
        localtime_r(&current_time, &lt);

        if (strftime(human_time, sizeof(human_time), "%x %X", &lt) == 0) {
                fprintf(stderr, "strftime error\n");
                return NULL;
        }

        printf("setting last sync time to %s\n", human_time);

        char *name_buffer = safe_calloc(512, 1);
        size_t i = 0;
        while (fgets(name_buffer, 512, list_fp) != NULL) {
                json_object *curr_metadata = json_object_array_get_idx(metadata, i);
                json_object *name_json;
                json_object_object_get_ex(curr_metadata, "Name", &name_json);
                const char *name = json_object_get_string(name_json);

                // remove newline from fgets()
                name_buffer[strlen(name_buffer) - 1] = 0;

                if (strcmp(name, name_buffer) != 0) {
                        printf("ERROR: metadata mismatch: %s -> %s", name_buffer, name);
                        continue;
                }

                json_object_object_add(
                                formatted_repodata,
                                name,
                                curr_metadata
                );

                i++;
        };

        return formatted_repodata;
}

int download_repos(void)
{
        char *listpath = packages_path;
        char *metapath = metadata_path;

        printf("Downloading repos...\n");

        const char *package_list_url = "https://aur.archlinux.org/packages.gz";
        const char *package_meta_url = "https://aur.archlinux.org/packages-meta-ext-v1.json.gz";

        size_t pkg_list_path_size = strlen(listpath) + 12;
        size_t pkg_meta_path_size = strlen(metapath) + 12;

        char *pkg_list_gz_path = safe_calloc(pkg_list_path_size, 1);
        char *pkg_meta_gz_path = safe_calloc(pkg_meta_path_size, 1);

        snprintf(pkg_list_gz_path, pkg_list_path_size, "%s.gz", listpath);
        snprintf(pkg_meta_gz_path, pkg_meta_path_size, "%s.gz", metapath);

        int rc1 = download_to_file(package_list_url, pkg_list_gz_path);
        int rc2 = download_to_file(package_meta_url, pkg_meta_gz_path);

        if (rc1 || rc2)
                return rc1;
        rc1 = gunzip(pkg_list_gz_path);
        rc2 = gunzip(pkg_meta_gz_path);
        if (rc1 || rc2)
                return 2;

        json_object *formatted_repodata = get_formatted_repo_data();

        if (json_object_to_file(repo_path, formatted_repodata) < 0)
                return 3;

        return 0;
}

int init_repo_data(void)
{
        json_object *j = json_object_from_file(repo_path);
        if (j == NULL) {
                /* couldn't read repo file */
                printf("failed to read repos, redownloading...\n");
                if (download_repos()) {
                        fprintf(stderr, "failed to re-download repos.\n");
                        return 1;
                }
                j = json_object_from_file(repo_path);
                if (j == NULL) {
                        fprintf(stderr, "failed to parse newly downloaded repos. possible serverside issue?\n");
                        return 1;
                }
        }

        repo_data = j;

        return 0;
}

bool package_is_installed(const char *name)
{
        if (name == NULL)
                return false;

        alpm_pkg_t *local_satisfier = alpm_find_satisfier(installed_packages,
                        name);
        if (local_satisfier == NULL)
                return false;
        return true;
}

json_object *get_aur_pkg_meta(const char *name)
{
        if (name == NULL)
                return NULL;

        json_object *meta;
        json_object_object_get_ex(repo_data, name, &meta);
        if (meta == NULL)
                return NULL;
        return meta;
}
