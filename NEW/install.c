#include "install.h"
#include "options.h"
#include "alloc.h"
#include "depend.h"
#include "repo.h"
#include "util.h"

#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <alpm_list.h>
#include <alpm.h>
#include <json-c/json.h>

int download_package_source(const char *name, struct opts *opts)
{
        size_t url_size = strlen(name) + 64;
        char *url = safe_calloc(url_size, 1);
        snprintf(url, url_size, "https://aur.archlinux.org/%s.git", name);
        printf("download package source from %s...\n", url);

        size_t download_path_len = strlen(opts->cache_path) + strlen(name) + 8;
        char *download_path = safe_calloc(download_path_len, 1);
        snprintf(download_path, download_path_len, "%s/%s",
                        opts->cache_path, name);

        char *argv[] = { "git", "clone", "--", url, download_path, NULL };
        if (execvp("git", argv) < 0) {
                perror("failed to clone");
                return 1;
        }

        free(url);
        free(download_path);

        return 0;
}

/*
 * package sources must already be downloaded.
 * THIS WILL NOT PROMPT FOR ANYTHING
 */
static int build_package(const char *name, struct opts *opts)
{
        json_object *package_data;
        json_object_object_get_ex(opts->repo_data, name, &package_data);

        /* not an aur package */
        if (package_data == NULL)
                return 1;

        return 0;
}

bool build_files_exist(const char *name, struct opts *opts)
{
        if (!dir_exists(opts->cache_path))
                return false;

        size_t build_path_len = strlen(name) + strlen(opts->cache_path) + 16;
        char *build_path = safe_calloc(build_path_len, 1);
        snprintf(build_path, build_path_len, "%s/%s", opts->cache_path, name);

        if (!dir_exists(build_path)) {
                free(build_path);
                return false;
        }

        /* directory, but no files */
        if (dir_is_empty(build_path)) {
                free(build_path);
                return false;
        }

        free(build_path);
        return true;
}

/*
 * Main function used to install packages.
 *
 * Will print packages, prompts for which packages to exclude
 * Prompts to install other AUR dependencies if needed
 * Asks to show diff (if build files exist) or MAKEPKG (if they don't)
 * Prints all GPG keys and prompts for importing
 */
int install_packages(const char **targets, int n, struct opts *opts)
{
        struct deplist *deps = deplist_new(n * 8);
        int n_aur_targets;

        struct deplist *aur_targets = install_dependencies(targets, n,
                        &n_aur_targets, deps, opts);
        if (aur_targets == NULL)
                fatal_err("failed to install dependencies");

        /* array of bools for whether build files downloaded */
        bool files_exist[n_aur_targets];
        json_object *meta_list[n_aur_targets];
        size_t longest_name_len = 0;

        for (int i = 0; i < n_aur_targets; i++) {
                size_t namelen = strlen(aur_targets->dl[i]->satisfier);
                if (namelen > longest_name_len)
                        longest_name_len = namelen;
        }

        for (int i = 0; i < n_aur_targets; i++) {
                struct dep *d = aur_targets->dl[i];
                char *target = d->satisfier;
                char *vers = d->vers;

                files_exist[i] = build_files_exist(target, opts);
                printf("aur/%s", target);

                size_t namelen = strlen(target);

                for (size_t j = 0; j < longest_name_len - namelen + 2; j++)
                        printf(" ");
                printf("%s", vers);

                if (files_exist[i])
                        printf(" (BUILD FILES EXIST)");
                printf("\n");
        }


        return 0;
}
