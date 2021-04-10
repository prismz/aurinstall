#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

typedef struct DirInfo {
    int exists;        /* whether or not the directory exists. This value should always be set.
                                 Always check this value before accessing any struct fields. */
    int filecount;     // amount of files in directory
    int error;         // whether or not an error occured while scanning. Will return zero if no errors occurred.

} DirInfo;

DirInfo dirinfo(char* path) {
    DirInfo x;
    DIR* dir = opendir(path);
    struct dirent *entry;

    if (dir) {
        x.exists = 1;
        for (int i = 0; ((entry = readdir(dir)) != NULL); i++)
            x.filecount++;

        closedir(dir);
    } else if (errno == ENOENT) {
        x.exists = 0;
        x.filecount = 0;
        x.error = 0;

    } else {
        x.error = 1;
        x.exists = 0;
        x.filecount = 0;
    }

    return x;
}