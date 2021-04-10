#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

char* cache_path = "~/.cache/aurinstall";

int install_package(char* name) {
    size_t package_path_size = sizeof(char) * strlen(cache_path)+90+strlen(name);
    char* package_path = malloc(package_path_size);
    snprintf(package_path, package_path_size, "%s/%s", cache_path, name);


    size_t clone_command_size = sizeof(char)*(100+strlen(name)*5+strlen(cache_path));
    char* clone_command = malloc(clone_command_size);
    snprintf(clone_command, clone_command_size, "git clone https://aur.archlinux.org/%s.git %s/%s", name, cache_path, name);

    int clone_return_code = system(clone_command);

    DirInfo package_path_info = dirinfo(package_path);
    if (package_path_info.exists && package_path_info.filecount > 0 && clone_command != 0) {
        char* input_str = malloc(sizeof(char) * 255);
        strcpy(input_str, "");
        printf(":: Files already exist for package %s. Would you like to rebuild the package? [Y/n] ", name);
        fgets(input_str, 255, stdin);

        if (!strcmp(input_str, "n")) {
            char* rmstr = malloc(sizeof(char)*(40+strlen(package_path)));
            strcpy(rmstr, "rm -rf ");
            strcat(rmstr, package_path);
            int rm_return_code = system(rmstr);
            if (rm_return_code != 0) {
                fprintf(stderr, "error: failed to remove all package files in directory %s.\n", package_path);
                free(package_path);
                free(clone_command);
                free(rmstr);
                free(input_str);
                return EXIT_FAILURE;
            }
            free(rmstr);
        }
        free(input_str);
    } else if (clone_return_code != 0) {
        fprintf(stderr, "error: non-zero return code from git-clone of package %s.\n", name);
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