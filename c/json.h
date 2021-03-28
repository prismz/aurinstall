#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_TO_STR(c) (char[2]){c, 0}

char* slice_str(char* str, int s, int e) {
    char* o = malloc(sizeof(str)+1);
    strcpy(o, "");
    for (int i = s; i < e; i++) {
        strcat(o, CHAR_TO_STR(str[i]));
    }
    return o;
}

// a really terrible way of parsing JSON but it works well enough
void parse_package_json(char _str[]) {
    char str[strlen(_str)+1];
    strcpy(str, _str);
    int current_parsing_type; // the type of item currently being parsed

    // for (int i = 0; i < strlen(str); i++) {
    //     char c = str[i];
    //     if (c == '{' || c == '[' || c == ',') {
    //         printf("%c\n", c);
    //     } else if (c == '}' || c == ']') {
    //         printf("\n%c", c);
    //     } else {
    //         printf("%c", c);
    //     }
    // }

    char* ptr = strtok(slice_str(str, 1, strlen(str)), ",");
    while (ptr != NULL) {
        printf("%s\n\n", slice_str(ptr, 1, strlen(ptr)-3));
        ptr = strtok(NULL, ",");
    }
    printf("\n");



}