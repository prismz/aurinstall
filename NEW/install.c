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
        json_object *package_data;
        json_object_object_get_ex(repo_data, name, &package_data);

        /* not an aur package */
        if (package_data == NULL)
                return 1;

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

/*
 * Main function used to install packages.
 *
 * Asks to show diff (if build files exist) or MAKEPKG (if they don't)
 * Prints all GPG keys and prompts for importing
 */
int install_packages(const char **targets, int n)
{
        printf("Installing dependencies...\n");
        struct deplist *aur_targets = install_dependencies(targets, n);
        int n_aur_targets = aur_targets->n;

        if (aur_targets == NULL)
                fatal_err("failed to install dependencies");

        /* array of bools for whether build files downloaded */
        bool files_exist[n_aur_targets];
        install_prompt(aur_targets, files_exist);

        /* download all sources */
        for (int i = 0; i < n_aur_targets; i++) {
                struct dep *target = aur_targets->dl[i];
                bool need_download = !files_exist[i];
                if (need_download)
                        download_package_source(target->satisfier);
        }

        printf("Importing PGP keys...\n");
        /* parse and import PGP keys */
        for (int i = 0; i < n_aur_targets; i++) {
                /* TODO: make keys->from not redundant, use one big list of keys
                 * that we can then print through */
                struct dep *target = aur_targets->dl[i];
                struct pgp_keylist *keys = get_pgp_keys(target->satisfier);
        }

        return 0;
}
