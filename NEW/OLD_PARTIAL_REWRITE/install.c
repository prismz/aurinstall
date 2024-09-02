#include "install.h"
#include "options.h"
#include "alloc.h"
#include "depend.h"
#include "util.h"
#include "srcinfo.h"

#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <alpm.h>
#include <json-c/json.h>

int download_package_source(const char *name)
{
        size_t url_size = strlen(name) + 64;
        char *url = safe_calloc(url_size, 1);
        snprintf(url, url_size, "https://aur.archlinux.org/%s.git", name);
        printf("download package source from %s...\n", url);

        size_t download_path_len = strlen(cache_path) + strlen(name) + 8;
        char *download_path = safe_calloc(download_path_len, 1);
        snprintf(download_path, download_path_len, "%s/%s",
                        cache_path, name);

        char *argv[] = { "git", "clone", "--", url, download_path, NULL };

        pid_t pid = fork();
        if (pid == -1) {
                fatal_err("failed to fork");
        } else if (pid > 0) {
                int ret;
                waitpid(pid, &ret, 0);
                if (ret != 0)
                        fatal_err("failed to download package sources");

                return 0;
        } else {
                execvp("git", argv);
                fatal_err("failed to download package sources");
        }

        return 0;
}

/*
 * package sources must already be downloaded.
 * THIS WILL NOT PROMPT FOR ANYTHING
 */
static int build_package(const char *name)
{
        /* TODO */
        return 0;
}

bool build_files_exist(const char *name)
{
        if (!dir_exists(cache_path))
                return false;

        size_t build_path_len = strlen(name) + strlen(cache_path) + 16;
        char *build_path = safe_calloc(build_path_len, 1);
        snprintf(build_path, build_path_len, "%s/%s", cache_path, name);

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
static void install_prompt(struct deplist *aur_targets, bool *files_exist)
{
        size_t longest_name_len = 0;
        size_t longest_vers_len = 0;
        int n = aur_targets->n;

        for (int i = 0; i < n; i++) {
                struct dep *target = aur_targets->dl[i];
                if (target == NULL)
                        continue;

                size_t namelen = strlen(target->satisfier);
                size_t verslen = strlen(target->vers);
                if (namelen > longest_name_len)
                        longest_name_len = namelen;
                if (verslen > longest_vers_len)
                        longest_vers_len = verslen;
        }

        for (int i = 0; i < n; i++) {
                struct dep *d = aur_targets->dl[i];
char *target = d->satisfier;
                char *vers = d->vers;

                files_exist[i] = build_files_exist(target);
                printf("aur/%s", target);

                size_t namelen = strlen(target);
                size_t verslen = strlen(vers);

                for (size_t j = 0; j < longest_name_len - namelen + 2; j++)
                        printf(" ");
                printf("%s", vers);

                for (size_t j = 0; j < longest_vers_len - verslen + 2; j++)
                        printf(" ");

                if (d->is_explicit)
                        printf(" (EXPLICIT)  ");
                else
                        printf(" (DEPENDENCY)");

                if (files_exist[i])
                        printf(" (BUILD FILES EXIST)");

                printf("\n");
        }
}
*/

/*
 * Main function used to install packages.
 *
 * Asks to show diff (if build files exist) or MAKEPKG (if they don't)
 * Prints all GPG keys and prompts for importing
 */
int install_packages(const char **targets, int n)
{
        struct stringlist *aur_deps = stringlist_new(n * 2);
        struct stringlist *aur_bdeps = stringlist_new(n * 2);
        struct stringlist *repo_deps = stringlist_new(n * 8);
        struct stringlist *repo_bdeps = stringlist_new(n * 8);
        struct stringlist *pgp_keys = stringlist_new(n * 4);

        printf(":: Downloading sources...\n");
        /* download all sources */
        for (int i = 0; i < n; i++) {
                const char *target = targets[i];
                alpm_pkg_t *repo_pkg = alpm_find_dbs_satisfier(alpm_handle,
                                sync_dbs, target);
                if (repo_pkg != NULL) {
                        printf("standard package %s, skipping (TODO)\n", target);
                        continue;
                }

                if (build_files_exist(target)) {
                        printf("source files exist for %s, skipping (TODO)\n", target);
                        continue;
                }

                download_package_source(target);
        }

        printf(":: Parsing SRCINFO files...\n");
        for (int i = 0; i < n; i++) {
                const char *target = targets[i];
                printf("::: Package %s\n", target);
                struct srcinfo *srcinfo = parse_pkg_srcinfo(target);

                parse_srcinfo_deps(srcinfo, &repo_deps, &repo_bdeps, &aur_deps,
                                &aur_bdeps, &pgp_keys);
        }

        return 0;
}
