#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "vars.h"

typedef struct DirInfo {
    int exists;        /* whether or not the directory exists. This value should always be set.
                                 Always check this value before accessing any struct fields. */
    int filecount;     // amount of files in directory

} DirInfo;

int dirinfo(char* path, DirInfo* info) {
    if (!info)
        return EXIT_FAILURE;
    DIR* dir = opendir(path);
    struct dirent *entry;

    if (dir) {
        info->exists = 1;
        for (int i = 0; ((entry = readdir(dir)) != NULL); i++)
            info->filecount++;

        closedir(dir);
    } else if (errno == ENOENT) {
        info->exists = 0;
        info->filecount = 0;

    } else {
        info->exists = 0;
        info->filecount = 0;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void init() {
    if (!cache_path) {
        char* home = getenv("HOME");
        size_t cache_size = sizeof(char) * strlen(home)+23;
        cache_path = malloc(cache_size);
        snprintf(cache_path, cache_size, "%s/.cache/aurinstall", home);
    }
}

void cleanup() {
    free(cache_path);
}

int is_confirmation(char* _str) {
    char* str = malloc(sizeof(char)*(strlen(_str)+1));
    strcpy(str, _str);
    for (int i = 0; str[i]!='\0'; i++) {
        if(str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] + 32;
        }
    }
    if (!strcmp(str, "y\n"))
        return 1;

    return 0;
}