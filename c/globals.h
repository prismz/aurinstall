#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <unistd.h>

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

int get_terminal_width() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
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

}