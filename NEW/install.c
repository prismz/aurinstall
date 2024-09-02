#include "install.h"
#include "options.h"
#include "util/util.h"
#include "util/target.h"
#include "util/alloc.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static int clone(const char *name)
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

static bool build_files_exist(const char *name)
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

static int update_build_files(const char *name)
{
        if (!dir_exists(cache_path))
                return 1;

        if (!build_files_exist(name))
                return clone(name);

        size_t build_path_len = strlen(name) + strlen(cache_path) + 16;
        char *build_path = safe_calloc(build_path_len, 1);
        snprintf(build_path, build_path_len, "%s/%s", cache_path, name);

        if (chdir(build_path) < 0) {
                free(build_path);
                return 1;
        }

        char *argv[] = { "git", "pull", "--", NULL };

        pid_t pid = fork();
        if (pid == -1) {
                fatal_err("failed to fork");
        } else if (pid > 0) {
                int ret;
                waitpid(pid, &ret, 0);
                if (ret != 0)
                        fatal_err("failed to update package sources");

                return 0;
        } else {
                execvp("git", argv);
                fatal_err("failed to update package sources");
        }

        return 0;

}

int clone_sources(struct target_list *targets)
{
        if (targets == NULL)
                return 1;
        if (targets->n < 1)
                return 0;

        for (int i = 0; i < targets->n; i++) {
                struct target *target = targets->targets[i];
                if (target->in_repos)
                        continue;
                struct target_list *deps = target->dependencies;
                if (clone_sources(deps))
                        return 2;

                if (!build_files_exist(target->name)) {
                        printf("Downloading sources for package %s\n",
                                        target->name);
                        if (clone(target->name))
                                return 3;
                } else {
                        printf("Updating sources for package %s\n",
                                        target->name);
                        if (update_build_files(target->name))
                                return 4;
                }
        }

        return 0;
}

/* doesn't recursively evaluate&build AUR dependencies, this is done by
 * build_and_install_dependencies() */
int install_repo_dependencies(struct target *target)
{
        if (target == NULL)
                return 1;
        if (target->dependencies->n == 0)
                return 0;

        char **argv = safe_calloc(target->dependencies->n + 8, sizeof(char *));
        argv[0] = root_program;
        argv[1] = "pacman";
        argv[2] = "-S";
        argv[3] = "--";

        int argv_i = 4;

        for (int i = 0; i < target->dependencies->n; i++) {
                struct target *dependency = target->dependencies->targets[i];

                /* skip AUR, this should be handled by build_and_install_dependencies() */
                if (!dependency->in_repos)
                        continue;

                argv[argv_i++] = dependency->name;
        }
        argv[argv_i] = NULL;

        pid_t pid = fork();
        if (pid == -1) {
                fatal_err("failed to fork");
        } else if (pid > 0) {
                int ret;
                waitpid(pid, &ret, 0);
                if (ret != 0)
                        fatal_err("failed to install repo dependencies");

                return 0;
        } else {
                execvp(root_program, argv);
                fatal_err("failed to install repo dependencies");
        }

        return 0;
}

/* just executes but properly handles fork() */
int exec_makepkg(void)
{
        char *argv[] = { "makepkg", "-s", "--", NULL };

        pid_t pid = fork();
        if (pid == -1) {
                fatal_err("failed to fork");
        } else if (pid > 0) {
                int ret;
                waitpid(pid, &ret, 0);
                if (ret != 0)
                        fatal_err("makepkg failed");

                return 0;
        } else {
                execvp(root_program, argv);
                fatal_err("makepkg failed");
        }

        return 0;
}

/* TODO: handle pacman packages as well */
int build_and_install_dependencies(struct target_list *targets)
{
        if (targets == NULL)
                return 1;

        size_t n_install_dirs = 0;
        size_t install_dirs_cap = 32;
        char **install_dirs = safe_calloc(install_dirs_cap, sizeof(char *));

        for (int i = 0; i < targets->n; i++) {
                struct target *target = targets->targets[i];
                if (target->dependencies->n != 0) {
                        install_repo_dependencies(target);
                        build_and_install_dependencies(target->dependencies);
                }

                for (int j = 0; j < target->dependencies->n; j++) {
                        struct target *depend = target->dependencies->targets[j];
                        if (depend->in_repos)
                                continue;

                        char *depend_path = path_join(cache_path, depend->name);

                        if (chdir(depend_path) < 0)
                                fatal_err("failed to cd into directory %s", depend_path);

                        if (exec_makepkg())
                                fatal_err("makepkg failed");

                        if (n_install_dirs + 1 > install_dirs_cap) {
                                install_dirs = safe_realloc(install_dirs,
                                                (install_dirs_cap += 8) *
                                                sizeof(char *));
                        }

                        install_dirs[n_install_dirs++] = depend_path;

                }
        }

        for (size_t i = 0; i < n_install_dirs; i++) {
                char *dir = install_dirs[i];
        }
}

/* TODO: .SRCINFO parsing, ability to view PKGBUILDs, color, better prompting */
int install_packages(char **targets, int n)
{
        if (targets == NULL)
                return 1;

        struct target_list *all_targets = get_target_list(targets, n);
        target_list_print(all_targets, 0);
        bool confirmed = yesno_prompt("install these packages?", true);
        if (!confirmed)
                return 0;

        printf("Downloading sources...\n");
        clone_sources(all_targets);

        printf("Recursively building AUR dependencies...\n");
}
