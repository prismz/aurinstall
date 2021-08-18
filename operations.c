/*
 * This file is part of aurinstall.
 *
 * aurinstall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * aurinstall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurinstall.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2021 Hasan Zahra
 * https://github.com/prismz/aurinstall
 */

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
 * npkg is a pointer to an integer representing the length of the returned array.
 *
 * max_name_len and max_ver_len are the maximum lengths of the name and version.
 * information_request is a pointer to an AUR api request, 
 * and each package returned with be appended.
 * request_len is the length of the information_request.
 *
 * call_again is a pointer to an integer which will be set to 1 
 * if there are more than 150 packages or if the length of the request 
 * to be sent to the AUR is too long, meaning the external function will have to
 * be called again to complete it's work.
 *
 * npkg is the only required argument. 
 * All others are optional and can be set to NULL.
 * The limit mentioned above will be disabled.
 */
struct package_data**
get_installed_packages(int* npkg, int* max_name_len,
                       int* max_ver_len, char* information_request,
                       size_t* request_len, int* call_again)
{
    FILE* fp = popen("pacman -Qm", "r");
    char* cbuff = smalloc(sizeof(char) * 1024);

    size_t n_allocated = 5;
    struct package_data** package_list = smalloc(
                sizeof(struct package_data) * n_allocated);
    int current_package = 0;

    while (fgets(cbuff, 1024, fp) != NULL) {
        char* cbuff_cpy = smalloc(sizeof(char) * 1024);
        strcpy(cbuff_cpy, cbuff);
        cbuff_cpy[strlen(cbuff_cpy) - 1] = '\0';

        char* ptr = strtok(cbuff_cpy, " ");
        char* pkg_name = smalloc(sizeof(char) * (strlen(ptr) + 1));
        strcpy(pkg_name, ptr);

        ptr = strtok(NULL, " ");
        char* pkg_ver = ptr;

        /*
         * limit of 150 packages and url length to 4443 characters due to
         * api request length limit. ignored if call_again is NULL
         */
        if (call_again != NULL && (current_package >= 149
            || *request_len + 20 + strlen(pkg_name) >= 4443)) {
            free(cbuff_cpy);
            free(pkg_name);
            *call_again = 1;
            break;
        }

        /* dynamically allocate package_list */
        if ((size_t)current_package - 1 > n_allocated)
            package_list = srealloc(package_list, 
                        sizeof(struct package_data) * (n_allocated *= 2));

        struct package_data* pi = smalloc(sizeof(struct package_data));
        pi->name = smalloc(sizeof(char) * (strlen(pkg_name) + 1));
        strcpy(pi->name, pkg_name);

        pi->ver = smalloc(sizeof(char) * (strlen(pkg_ver) + 1));
        strcpy(pi->ver, pkg_ver);
        pi->desc = NULL;
        pi->ood = NULL;

        if (information_request != NULL && request_len != NULL) {
            strcat(information_request, "&arg[]=");
            strcat(information_request, pkg_name);
            *request_len = strlen(information_request);
        }

        package_list[current_package] = pi;

        /* used for formatting later */
        if (max_name_len != NULL && strlen(pkg_name) > (size_t)*max_name_len)
            *max_name_len = strlen(pkg_name);

        if (max_ver_len != NULL && strlen(pkg_ver) > (size_t)*max_ver_len)
            *max_ver_len = strlen(pkg_ver);

        current_package++;

        sfree(pkg_name);
        sfree(cbuff_cpy);
    }
    sfree(cbuff);
    fclose(fp);
    *npkg = current_package;

    return package_list;
}

/*
 * will ignore first item in searchterms
 * because the first item is the keyword "search".
 */
void
search_aur(char** searchterms, size_t searchterm_count)
{
    int normal_tty = isatty(STDOUT_FILENO);

    /* build api request string, add 60 for space for url */
    size_t request_str_size = sizeof(char) * (strlen(searchterms[1]) + 60);
    char* request_str = smalloc(request_str_size);
    snprintf(request_str, request_str_size, 
                "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s", searchterms[1]);
    struct curl_str cs = requests_get(request_str);
    sfree(request_str);

    struct api_results* ar = parse_api_results(&cs);
    int resultcount = ar->resultcount;
    if (ar->error != NULL)
        fprintf(stderr, "error: %s\n", ar->error);

    sfree(cs.ptr);

    int n_installed_packages = 0;
    struct package_data** installed_packages = get_installed_packages(
                &n_installed_packages, NULL, NULL, NULL, NULL, NULL);

    for (int i = 0; i < resultcount; i++) {
        struct json_object* jobj_current_package = json_object_array_get_idx(
                        ar->results, i);
        struct package_data* pi = parse_package_data(jobj_current_package);
        char* name = pi->name;
        char* desc = pi->desc;
        char* ver = pi->ver;
        char* ood = pi->ood;
        int installed = pkg_array_contains(installed_packages, 
                    n_installed_packages, name);

        int pkg_valid = 1;
        /* 
         * start at index 2 because keyword "search" is 
         * typically included in argument list. 
         */
        for (int j = 2; (size_t)j < searchterm_count; j++) {
            if ((name != NULL && strstr(name, searchterms[j]) == NULL) &&
                (desc != NULL && strstr(desc, searchterms[j]) == NULL)) {
                pkg_valid = 0;
                break;
            }
        }

        if (!pkg_valid) {
            free_package_data(pi);
            continue;
        }

        if (normal_tty) {  /* print in color */
            printf("%s%saur/%s%s%s %s%s%s%s", BOLD, RED, ENDC, name, ENDC, ENDC, 
                        GREEN, ver, ENDC);
            if (ood != NULL)
                printf(" %s(OUT OF DATE)%s", RED, ENDC);
            if (installed)
                printf(" %s[INSTALLED]%s", BLUE, ENDC);

            printf("\n");
            if (desc != NULL)
                pretty_print((char*)desc, 4);
        } else {
            printf("aur/%s %s", name, ver);
            if (ood != NULL)
                printf(" (OUT OF DATE)");
            if (installed)
                printf(" [INSTALLED]");

            printf("\n");
            if (desc != NULL)
                printf("    %s\n", desc);
        }
        free_package_data(pi);
    }
    free_api_results(ar);

    for (int i = 0; i < n_installed_packages; i++)
        free_package_data(installed_packages[i]);
    sfree(installed_packages);
}

void
install_aur_package(char* name, char* cache_dir)
{
    char* package_dest = smalloc(sizeof(char) * 256);
    snprintf(package_dest, 256, "%s/%s", cache_dir, name);

    printf("%s\n", package_dest);

    char* info_str = smalloc(sizeof(char) * (strlen(name) + 512));
    snprintf(info_str, strlen(name) + 512, 
                "https://aur.archlinux.org/rpc/?v=5&type=info&arg=%s", name);

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
        char* op = smalloc(sizeof(char) * 1024);
        fgets(op, 1024, stdin);

        if (strcmp(op, "y\n")) {
            free_package_data(pi);
            free_api_results(ar);
            sfree(op);
            sfree(package_dest);
            return;
        }
        sfree(op);
    }
    free_package_data(pi);

    int cloned = 0;
    if (!dir_is_empty(package_dest)) {
        cloned = 1;
        printf("cache directory for package %s is not empty. Would you like to rebuild the package? [y/N] ", name);
        char* op = smalloc(sizeof(char) * 1024);
        fgets(op, 1024, stdin);

        if (!strcmp(op, "y\n")) {
            snsystem(300, "rm -rf %s", package_dest);
            cloned = 0;
        }
        
        sfree(op);
    }

    if (!cloned) {
        int git_clone_r = snsystem(strlen(name) * 2 + strlen(cache_dir) + 512, 
            "git clone https://aur.archlinux.org/%s.git %s/%s", name, cache_dir, name);
        
        if (git_clone_r != EXIT_SUCCESS) {
            fprintf(stderr, "failed to clone package %s.\n", name);
            sfree(package_dest);
            return;
        }
    }

    int makepkg_c = snsystem(strlen(cache_dir) + 1024, 
                "cd %s && makepkg -si", package_dest);

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

    int check = system("pacman -Qm &> /dev/null");
    if (check != EXIT_SUCCESS) {
        fprintf(stderr, "failed to get info for installed AUR packages.");
        return 0;
    }

    char* information_request = smalloc(sizeof(char) * 4444);  /* 4443 + 1 */
    strcpy(information_request, "https://aur.archlinux.org/rpc/?v=5&type=info");
    size_t request_len = strlen(information_request);

    int current_package = 0;
    int max_name_len = 0;
    int max_ver_len = 0;
    struct package_data** package_list = get_installed_packages(
                    &current_package, &max_name_len, &max_ver_len,
                    information_request, &request_len, &call_again);

    /* indexes of packages to update */
    int packages_to_update[current_package+10];
    int cpkg_to_update = 0;

    struct curl_str cs = requests_get(information_request);
    struct api_results* ar = parse_api_results(&cs);

    sfree(information_request);
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
        struct json_object* jobj_current_result = json_object_array_get_idx(
                    ar->results, i);
        struct package_data* pi = parse_package_data(jobj_current_result);

        char* cpkg_pn = package_list[i]->name;
        char* cpkg_pv = package_list[i]->ver;

        right_pad_print_str(pi->name, max_name_len, 3);
        if (cpkg_pv != NULL)
            right_pad_print_str(cpkg_pv,  max_ver_len, 3);
        else
            right_pad_print_str("(none)", max_ver_len, 3);

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

        snsystem(500, "rm -rf %s/%s", cache_dir, package_list[package_index]->name);

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
        fprintf(stderr, 
                    "error: could not read package cache directory %s.\n", cache_dir);
        return;
    }
    while ((d = readdir(dir)) != NULL) {
        char* d_name = d->d_name;
        if (!strcmp(d_name, ".") || !strcmp(d_name, ".."))
            continue;
        printf("clean cache for package %s", d_name);

        snsystem(300, "rm -rfv %s/%s", cache_dir, d_name);

        printf(" - done\n");
    }
    closedir(dir);
    printf("done.\n");
}

void
remove_packages(char* packages)
{
    snsystem(strlen(packages) + 50, "sudo pacman -R %s", packages);
}