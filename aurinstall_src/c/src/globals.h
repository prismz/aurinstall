#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <unistd.h>

#define MAX_ARGLEN 1096

const char* BOLD = "\033[1m";
const char* GREEN = "\033[92m";
const char* RED = "\033[91m";
const char* ENDC = "\033[0m";

struct curl_res_string {
    char *ptr;
    size_t len;
};

typedef struct PackageData {
    char* name;
    char* desc;
    char* ver;
    char* url;
    char* ood;
} PackageData;

typedef struct Options {
    int normal_term;
} Options;

void free_package_data(PackageData data) {
    free(data.name);
    free(data.desc);
    free(data.ver);
    free(data.url);
    free(data.ood);
}

// json results typically return with quotes, so this function removes them.
void remquotes(char *str) {
    size_t len = strlen(str);
    if (len <= 2)
        return;

    if (str[0] != '"' || str[len-1] != '"')
        return;

    memmove(str, str+1, len-2);
    str[len-2] = 0;
}

int get_terminal_width(Options* opts) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    
    return (int)w.ws_col;
}

void pretty_print(int indent, char data[], Options* opts) {
    int slen = strlen(data);
    int termwidth = get_terminal_width(opts) - indent;

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

void init(Options* opts) {
    // if output is not being piped or redirected, meaning terminal should show color output
    if (isatty(STDOUT_FILENO)) {
        opts->normal_term = 1;
    } else
        opts->normal_term = 0;
}