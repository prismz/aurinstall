#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "vars.h"

#include "simple-json/simple_json.h"


int install_package(char* name) {
    size_t package_path_size = sizeof(char) * strlen(cache_path)+90+strlen(name);
    char* package_path = malloc(package_path_size);
    snprintf(package_path, package_path_size, "%s/%s", cache_path, name);

    size_t clone_command_size = sizeof(char)*(100+strlen(name)*5+strlen(cache_path));
    char* clone_command = malloc(clone_command_size);
    snprintf(clone_command, clone_command_size, "git clone https://aur.archlinux.org/%s.git %s/%s", name, cache_path, name);

    int clone_return_code = system(clone_command);

    DirInfo package_path_info;
    dirinfo(package_path, &package_path_info);

    if (package_path_info.exists && package_path_info.filecount > 0 && clone_command != 0) {
        char* input_str = malloc(sizeof(char) * 255);
        strcpy(input_str, "");
        printf(":: Files already exist for package %s. Would you like to rebuild the package? [y/N] ", name);
        fgets(input_str, 255, stdin);

        if (is_confirmation(input_str)) {
            size_t rmstr_size = sizeof(char)*(40+strlen(package_path));
            char* rmstr = malloc(rmstr_size);
            snprintf(rmstr, rmstr_size, "rm -rf %s", package_path);

            int rm_return_code = system(rmstr);
            if (rm_return_code != 0) {
                fprintf(stderr, "error: failed to remove all package files in directory %s.\n", package_path);
                free(package_path);
                free(clone_command);
                free(rmstr);
                free(input_str);

                return EXIT_FAILURE;
            }

            int c_clone_return_code = system(clone_command);
            if (c_clone_return_code != 0) {
                fprintf(stderr, "error: error cloning package %s from AUR.\n", name);
                return EXIT_FAILURE;
            }
            free(rmstr);
        }
        free(input_str);

    } else if (clone_return_code != 0) {
        fprintf(stderr, "error: non-zero return code from git-clone of package %s. Try deleting %s.\n", name, package_path);
        return EXIT_FAILURE;
    }

    size_t makepkg_command_size = sizeof(char) * (200 + strlen(package_path));
    char* makepkg_command = malloc(makepkg_command_size);
    snprintf(makepkg_command, makepkg_command_size, "cd %s && makepkg -si", package_path);

    int makepkg_returncode = system(makepkg_command);
    if (makepkg_returncode != 0) {
        fprintf(stderr, "error: non-zero return code from makepkg for package %s.\n", name);
    }

    free(package_path);
    free(clone_command);
}

void display_package_json(char* json) {

}

/* 
trys to have the fastest method of searching by using the logic that the longer
a searchterm is, the less results it will have.
*/
void search_package(char* searchterms[SEARCH_MAX_ARG_COUNT], int termcount) {
    char* base_search_url = "https://aur.archlinux.org/rpc/?v=5&type=search&arg=";
    // int lengths[termcount];
    if (termcount == 1) {
        size_t search_api_str_size = sizeof(char) * 53 + strlen(searchterms[0]);
        char* search_api_str = malloc(search_api_str_size);
        snprintf(search_api_str, search_api_str_size, "%s%s", base_search_url, searchterms[0]);
        struct curl_res_string res = get(search_api_str);

        char* resultcount_ = json_parse_dict(res.ptr, "results");
        char* nptr;
        int resultcount = strtol(resultcount_, &nptr, 0);
        printf("%d\n", resultcount);
        // display_package_json(res.ptr);
    }
    // for (int i = 0; i < termcount; i++) {
    //     lengths[i] = strlen(searchterms[i]);
    // }
}