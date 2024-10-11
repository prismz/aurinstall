#include "repo.h"
#include "util.h"
#include "alloc.h"
#include "string.h"
#include "requests.h"
#include "fastdeps.h"
#include "../options.h"

#include <alpm.h>
#include <string.h>
#include <time.h>
#include <json-c/json.h>

json_object *format_repo(const char *path)
{
        json_object *unformatted = json_object_from_file(path);
        if (unformatted == NULL)
                return NULL;
        size_t unformatted_n = json_object_array_length(unformatted);

        json_object *formatted = json_object_new_object();
        for (size_t i = 0; i < unformatted_n; i++) {
                json_object *ent = json_object_array_get_idx(unformatted, i);

                json_object *name_json;
                json_object_object_get_ex(ent, "Name", &name_json);
                if (name_json == NULL)
                        continue;
                const char *name = json_object_get_string(name_json);

                json_object_object_add(formatted, name, ent);
        }

        if (json_object_to_file(path, formatted))
                return NULL;

        repo_data = formatted;

        return formatted;
}

int download_repos(void)
{
        printf("Downloading repos...\n");

        const char *db_url = "https://aur.archlinux.org/packages-meta-ext-v1.json.gz";

        size_t repo_gz_path_len = strlen(repo_path) + 12;
        char *repo_gz_path = safe_calloc(repo_gz_path_len, 1);
        snprintf(repo_gz_path, repo_gz_path_len, "%s.gz", repo_path);

        int rc1 = download_to_file(db_url, repo_gz_path);
        if (rc1)
                return rc1;

        rc1 = gunzip(repo_gz_path);
        if (rc1)
                return 2;

        json_object *formatted_repodata = format_repo(repo_path);
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
                if (repo_data == NULL) {
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
