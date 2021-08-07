#include "operations.h"
#include "util.h"
#include "mem.h"
#include "rpc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <json-c/json.h>

/*
 * will ignore first item in searchterms
 * because the first item is the keyword "search".
 */
void
search_aur(char** searchterms, size_t searchterm_count)
{
    int normal_tty = isatty(STDOUT_FILENO);

    /* build api request string */
    char* request_str = smalloc(strlen(searchterms[1]) + 350, "request_str - search_aur() - operations.c");
    snprintf(request_str, strlen(searchterms[1]) + 350, "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s", searchterms[1]);
    struct curl_str cs = requests_get(request_str);
    sfree(request_str);

    struct api_results* ar = parse_api_results(&cs);
    int resultcount = ar->resultcount;
    sfree(cs.ptr);

    for (int i = 0; i < resultcount; i++) {
        struct json_object* jobj_current_package = json_object_array_get_idx(ar->results, i);
        struct package_data* pi = parse_package_data(jobj_current_package);
        char* name = pi->name;
        char* desc = pi->desc;
        char* ver = pi->ver;
        char* ood = pi->ood;

        int pkg_valid = 1;
        for (int j = 2; (size_t)j < searchterm_count; j++) { /* start at index 2 because keyword "search" is typically included in argument list. */
            if (name != NULL && strstr(name, searchterms[j]) == NULL) {
                pkg_valid = 0;
                break;
            }
        }

        if (!pkg_valid) {
            free_package_data(pi);
            continue;
        }

        if (normal_tty) {  /* print in color */
            printf("%s%saur/%s%s%s %s%s%s", BOLD, RED, ENDC, name, ENDC, GREEN, ver, ENDC);
            if (ood != NULL)
                printf(" %s(OUT OF DATE)%s", RED, ENDC);
            printf("\n");
            if (desc != NULL)
                pretty_print((char*)desc, 4);
        } else {
            printf("aur/%s %s", name, ver);
            if (ood != NULL)
                printf(" (OUT OF DATE)");
            printf("\n");
            if (desc != NULL)
                printf("    %s\n", desc);
        }
        free_package_data(pi);
    }

    free_api_results(ar);
}

void
install_aur_package(char* name, char* cache_dir)
{
    char* package_dest = smalloc(sizeof(char) * 256, "package_dest - install_aur_package() - operations.c");
    snprintf(package_dest, 256, "%s/%s", cache_dir, name);

    printf("%s\n", package_dest);

    char* info_str = smalloc(sizeof(char) * strlen(name) + 512, "info_str - install_aur_package() - operations.c");
    snprintf(info_str, strlen(name) + 512, "https://aur.archlinux.org/rpc/?v=5&type=info&arg=%s", name);

    struct curl_str cs = requests_get(info_str);
    sfree(info_str);

    struct api_results* ar = parse_api_results(&cs);
    sfree(cs.ptr);

    if (ar->resultcount != 1) {
        fprintf(stderr, "error: could not get information for package %s.\n", name);
        sfree(package_dest);
        free_api_results(ar);
        return;
    }

    /* check if package is out of date */
    json_object* jobj_pkgres = json_object_array_get_idx(ar->results, 0);
    struct package_data* pi = parse_package_data(jobj_pkgres);

    if (pi->ood != NULL) {
        printf("package %s is out of date. Would you like to continue installing the package? [y/N] ", name);
        char* op = smalloc(sizeof(char) * 1024, "op - install_aur_package() - operations.c - (out of date y/n)");
        fgets(op, 1024, stdin);

        if (strcmp(op, "y\n")) {
            free_package_data(pi);
            free_api_results(ar);
            sfree(op);
            sfree(package_dest);
            sfree(info_str);
            return;
        }
        sfree(op);
    }
    free_package_data(pi);

    int cloned = 0;
    if (!dir_is_empty(package_dest)) {
        cloned = 1;
        printf("cache directory for package %s is not empty. Would you like to rebuild the package? [y/N] ", name);
        char* op = smalloc(sizeof(char) * 1024, "op - install_aur_package() - operations.c - (clear cache y/n)");
        fgets(op, 1024, stdin);

        if (!strcmp(op, "y\n")) {
            char* rm_cmd = smalloc(sizeof(char) * 300, "rm_cmd - install_aur_package() - operations.c - (clear package cache dir)");
            snprintf(rm_cmd, 300, "rm -rf %s", package_dest);

            system(rm_cmd);
            sfree(rm_cmd);
        }
        sfree(op);
    }

    if (!cloned) {
        size_t git_clone_command_size = (strlen(name) * 3) + strlen(cache_dir) + 512;
        char* git_clone_command = smalloc(sizeof(char) * git_clone_command_size, "git_clone_command - install_aur_package() - operations.c");
        snprintf(git_clone_command, git_clone_command_size, "git clone https://aur.archlinux.org/%s.git %s/%s", name, cache_dir, name);
        int git_clone_r = system(git_clone_command);
        sfree(git_clone_command);
        if (git_clone_r != EXIT_SUCCESS) {
            fprintf(stderr, "failed to clone package %s.\n", name);
            sfree(package_dest);
            return;
        }
    }

    size_t makepkg_cmd_size = strlen(cache_dir) + 1024;
    char* makepkg_cmd = smalloc(sizeof(char) * makepkg_cmd_size, "makepkg_cmd - install_aur_package() - operations.c");
    snprintf(makepkg_cmd, makepkg_cmd_size, "cd %s && makepkg -si", package_dest);
    sfree(package_dest);

    printf("begin makepkg\n");
    int makepkg_c = system(makepkg_cmd);
    sfree(makepkg_cmd);

    if (makepkg_c != EXIT_SUCCESS) {
        fprintf(stderr, "error: makepkg for package %s failed.\n", name);
        return;
    }
}

/*
 * request can be no more than 4443 bytes.
 * info requests with more than 150 packages need to be split.
 * returns 1 if needs to be called again to fully update
 */
int
update_installed_packages(char* cache_dir)
{
    /*
     * if there are more than 150 packages,
     * run the update function until all packages are updated.
     */
    int call_again = 0;

    FILE* fp = popen("pacman -Qm", "r");
    char* cbuff = smalloc(sizeof(char) * 1024, "cbuff - update_installed_packages() - operations.c");

    char* information_request = smalloc(sizeof(char) * 4444, "information_request - update_installed_packages() - operations.c");  /* 4443 + 1 */
    strcpy(information_request, "https://aur.archlinux.org/rpc/?v=5&type=info");
    size_t request_len = strlen(information_request);

    struct package_data** package_list = smalloc(sizeof(struct package_data) * 151, "package_list - update_installed_packages() - operations.c");
    int current_package = 0;

    int max_name_len = 10;
    int max_ver_len = 7;

    while (fgets(cbuff, 1024, fp) != NULL) {
        char* cbuff_cpy = smalloc(1024, "cbuff_cpy - update_installed_packages() - operations.c");
        strcpy(cbuff_cpy, cbuff);
        cbuff_cpy[strlen(cbuff_cpy) - 1] = '\0';

        char* ptr = strtok(cbuff_cpy, " ");
        char* pkg_name = smalloc(strlen(ptr) + 1, "pkg_name - update_installed_packages() - operations.c");
        strcpy(pkg_name, ptr);

        ptr = strtok(NULL, " ");
        char* pkg_ver = ptr;

        if (current_package >= 149 || (request_len + 20 + strlen(pkg_name)) >= 4443) {
            call_again = 1;
            break;
        }

        struct package_data* pi = smalloc(sizeof(struct package_data), "pi - update_installed_packages() - operations.c - pacman output");
        pi->name = smalloc(sizeof(char) * (strlen(pkg_name) + 1), "pi->name - update_installed_packages() - operations.c - pacman output");
        strcpy(pi->name, pkg_name);

        pi->ver = smalloc(sizeof(char) * (strlen(pkg_ver) + 1), "pi->ver - update_installed_packages() - operations.c");
        strcpy(pi->ver, pkg_ver);
        pi->desc = NULL;
        pi->ood = NULL;

        strcat(information_request, "&arg[]=");
        strcat(information_request, pkg_name);
        request_len = strlen(information_request);

        package_list[current_package] = pi;

        /* used for formatting later */
        if (strlen(pkg_name) > (size_t)max_name_len)
            max_name_len = strlen(pkg_name);

        if (strlen(pkg_ver) > (size_t)max_ver_len)
            max_ver_len = strlen(pkg_ver);

        current_package++;

        sfree(pkg_name);
        sfree(cbuff_cpy);
    }
    sfree(cbuff);
    fclose(fp);

    /* indexes of packages to update */
    int packages_to_update[current_package+10];
    int cpkg_to_update = 0;

    struct curl_str cs = requests_get(information_request);
    sfree(information_request);

    struct api_results* ar = parse_api_results(&cs);
    sfree(cs.ptr);

    if (ar->resultcount <= 0) {
        fprintf(stderr, "error: no results.\n");
        for (int i = 0; i < current_package; i++)
            free_package_data(package_list[i]);

        sfree(package_list);
        return 0;
    }

    right_pad_print_str("package", max_name_len, 3);
    right_pad_print_str("current", max_ver_len, 3);
    right_pad_print_str("latest", max_ver_len, 3);
    printf("\n");

    for (int i = 0; i < ar->resultcount; i++) {
        struct json_object* jobj_current_result = json_object_array_get_idx(ar->results, i);
        struct package_data* pi = parse_package_data(jobj_current_result);

        char* cpkg_pn = package_list[i]->name;
        char* cpkg_pv = package_list[i]->ver;

        right_pad_print_str(pi->name, max_name_len, 3);
        right_pad_print_str(cpkg_pv,  max_ver_len, 3);
        right_pad_print_str(pi->ver,  max_ver_len, 3);

        if (!strcmp(pi->name, cpkg_pn) && strcmp(pi->ver, cpkg_pv)) {
            packages_to_update[cpkg_to_update] = i;
            cpkg_to_update++;
        }
        printf("\n");
        free_package_data(pi);
    }
    free_api_results(ar);

    if (cpkg_to_update == 0)
        printf("no updates.\n");

    for (int i = 0; i < cpkg_to_update; i++) {
        int package_index = packages_to_update[i];

        char* rm_pkg_cache_path = smalloc(266, "rm_pkg_cache_path - update_installed_packages() - operations.c");
        snprintf(rm_pkg_cache_path, 256, "rm -rf %s/%s", cache_dir, package_list[package_index]->name);
        system(rm_pkg_cache_path);
        sfree(rm_pkg_cache_path);

        install_aur_package(package_list[package_index]->name, cache_dir);
    }

    for (int i = 0; i < current_package; i++)
        free_package_data(package_list[i]);

    sfree(package_list);

    return call_again;
}

void
clean_package_cache(char* cache_dir)
{
    struct dirent* d;
    DIR* dir = opendir(cache_dir);
    if (!dir) {
        fprintf(stderr, "error: could not read package cache directory %s.\n", cache_dir);
        return;
    }
    while ((d = readdir(dir)) != NULL) {
        char* d_name = d->d_name;
        if (!strcmp(d_name, ".") || !strcmp(d_name, ".."))
            continue;
        printf("clean cache for package %s", d_name);
        char* rm_cmd = smalloc(sizeof(char) * 300, "rm_cmd - clean_package_cache() - operations.c");
        snprintf(rm_cmd, 300, "rm -rf %s/%s", cache_dir, d_name);
        system(rm_cmd);
        sfree(rm_cmd);
        printf(" - done\n");
    }
    printf("done.\n");
}

void
remove_package(char* name)
{
    size_t cmd_size = strlen(name) + 50;
    char* cmd = smalloc(sizeof(char) * cmd_size, "cmd - remove_package() - operations.c");
    snprintf(cmd, cmd_size, "sudo pacman -R %s", name);
    system(cmd);
    sfree(cmd);
}
