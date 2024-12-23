#include "install.h"
#include "options.h"
#include "util/util.h"
#include "util/fastdeps.h"
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

int clone_sources(TargetList *targets)
{
        if (targets == NULL)
                return 1;
        if (targets->n < 1)
                return 0;

        for (int i = 0; i < targets->n; i++) {
                Target *t = targets->targets[i];
                if (!t->aur)
                        continue;
                TargetList *deps = t->depends;
                TargetList *mdeps = t->makedepends;
                if (clone_sources(deps))
                        return 2;
                if (clone_sources(mdeps))
                        return 3;

                if (!build_files_exist(t->name)) {
                        printf("Downloading sources for package %s\n",
                                        t->name);
                        if (clone(t->name))
                                return 4;
                } else {
                        printf("Updating sources for package %s\n",
                                        t->name);
                        if (update_build_files(t->name))
                                return 5;
                }
        }

        return 0;
}

/* doesn't recursively evaluate&build AUR dependencies, this is done by
 * build_and_install_dependencies() */
int install_repo_dependencies(Target *target)
{
        if (target == NULL)
                return 1;
        if (target->depends->n == 0 && target->makedepends->n == 0)
                return 0;

        char **argv = safe_calloc(target->depends->n + target->makedepends->n + 8, sizeof(char *));
        argv[0] = root_program;
        argv[1] = "pacman";
        argv[2] = "-S";
        argv[3] = "--asdeps";
        argv[4] = "--";

        int argv_i = 5;

        for (int i = 0; i < target->depends->n; i++) {
                Target *dependency = target->depends->targets[i];

                /* skip AUR, this should be handled by build_and_install_dependencies() */
                if (dependency->aur)
                        continue;

                argv[argv_i++] = dependency->name;
        }

        for (int i = 0; i < target->makedepends->n; i++) {
                Target *dependency = target->makedepends->targets[i];
                if (dependency->aur)
                        continue;
        }

        argv[argv_i] = NULL;
        if (argv_i == 5) {  /* no arguments */
                printf("no repo dependencies to install.\n");
                return 0;
        }

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
        /* TODO: makepkg --packagelist */
        char *argv[] = { "makepkg", "-sf", "--", NULL };

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
                execvp("makepkg", argv);
                fatal_err("makepkg failed");
        }

        return 0;
}

/* TODO: handle pacman packages as well */
int build_and_install_dependencies(TargetList *targets)
{
        if (targets == NULL)
                return 1;

        size_t n_install_dirs = 0;
        size_t install_dirs_cap = 32;
        char **install_dirs = safe_calloc(install_dirs_cap, sizeof(char *));

        for (int i = 0; i < targets->n; i++) {
                Target *target = targets->targets[i];

                printf("process %s\n", target->name);
                if (target->depends->n != 0) {
                        install_repo_dependencies(target);
                        build_and_install_dependencies(target->depends);
                }

                for (int j = 0; j < target->depends->n; j++) {
                        Target *depend = target->depends->targets[j];
                        if (!depend->aur)
                                continue;

                        char *depend_path = path_join(cache_path, depend->name);

                        if (chdir(depend_path) < 0)
                                fatal_err("failed to cd into directory %s", depend_path);

                        printf("running makepkg\n");
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
                printf("Installing in dir %s\n", dir);
        }

        return 0;
}

/* TODO: .SRCINFO parsing, ability to view PKGBUILDs, color, better prompting */
int install_packages(const char **targets, int n)
{
        if (targets == NULL)
                return 1;

        TargetList *all_targets = get_target_list(targets, n);
        print_targetlist(all_targets);
        bool confirmed = yesno_prompt("install these packages?", true);
        if (!confirmed)
                return 0;

        printf("Downloading sources...\n");
        clone_sources(all_targets);

        printf("Recursively building AUR dependencies...\n");
        build_and_install_dependencies(all_targets);
}
