#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>

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


// json results typically return with quotes, so this function removes them.
void remquotes(char* str) {
    size_t len = strlen(str);
    if (len <= 2)
        return;

    if (str[0] != '"' || str[len-1] != '"')
        return;

    memmove(str, str+1, len-2);
    str[len-2] = 0;
}

int get_terminal_width() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    
    return (int)w.ws_col;
}

void pretty_print(int indent, char data[]) {
    int slen = strlen(data);
    int termwidth = get_terminal_width() - indent;

    char words[slen][slen];
    int wordcount = 0;
    int printed = 0;
    
    char* ptr = strtok(data, " ");
    while (ptr != NULL) {
        strcpy(words[wordcount], ptr);
        ptr = strtok(NULL, " ");
        wordcount++;
    }
    for (int i = 0; i < indent; i++)
        printf(" ");

    for (int i = 0; i < wordcount; i++) {
        char* word = words[i];
        printf("%s ", word);
        printed += strlen(word)+1;

        if (i == wordcount - 1) {
            break;
        }
        if ((printed + strlen(words[i+1]) + 1) >= termwidth) {
            printf("\n");
            for (int i = 0; i < indent; i++)
                printf(" ");
            printed = 0;
        }
    }
    printf("\n");
    for (int i = 0; i < wordcount; i++)
        strcpy(words[i], "");
}


void init() {
    if (!cache_path) {
        char* home = getenv("HOME");
        size_t cache_size = sizeof(char) * strlen(home)+23;
        cache_path = malloc(cache_size);
        snprintf(cache_path, cache_size, "%s/.cache/aurinstall", home);
    }

    if (!isatty(STDOUT_FILENO))
        normal_tty = 0;
    else 
        normal_tty = 1;
}

void cleanup() {
    free(cache_path);
}
