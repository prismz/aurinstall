/*
    simple-json - A lightweight JSON parser written in C - github.com/HasanQZ/simple-json
    Copyright (C) 2021  Hasan A. Zahra.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see https://www.gnu.org/licenses/gpl-3.0.html.
*/
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_TO_STR(c) (char[2]){c, 0}

size_t mallocated = 0;

void* s_malloc(size_t s) {
    mallocated += s;
    void* c = malloc(s);
    if (c == NULL) {
        fprintf(stderr, "OUT OF MEMORY!");
        exit(1);
    }
    return c;
}

void s_free(void* tgt) {
    mallocated -= sizeof(tgt);
    free(tgt);
}

char* json_parse_dict(char str[], char _key[]) {
    int strl = strlen(str);
    int in_string = 0;
    int on_target = 0;
    int depth = 0;
    int s_offset = 0;
    int e_offset = 0;

    char* key = s_malloc(sizeof(char) * (int)(strlen(_key)*1.5));
    strcpy(key, "\"");
    strcat(key, _key);
    strcat(key, "\"");

    // char* value = s_malloc(sizeof(char) * (strlen(str)+5));
    char* current_str = s_malloc(sizeof(char) * strlen(str));
    int current_str_i = 0;
    strcpy(current_str, "");

    for (int i = 0; i < strl; i++) {
        char c = str[i];
        if (c == ' ' || c == '\n' || c == '\t') {
            s_offset++;
        } else
            break;
    }

    for (int i = strl-1; i >= 0; i--) {
        char c = str[i];
        if (c == ' ' || c == '\n' || c == '\t') {
            e_offset++;
        } else
            break;
    }

    for (int i = s_offset+1; i < strl-e_offset; i++) {
        char c = str[i];
        if (c == '"')
            in_string = !in_string;

        if (!in_string) {
            if (c == ' ' || c == '\n' || c == '\t')
                continue;

            if (c == '[' || c == '{')
                depth++;

            else if (c == ']' || (c == '}' && i != strl-1))
                depth--;

            if (c == ':' && depth == 0) {
                if (!strcmp(current_str, key)) {
                    on_target = 1;
                } else {
                    on_target = 0;
                }

                strcpy(current_str, "");
                current_str_i = 0;
            }
            else if ((c == ',' && depth == 0) || (c == '}' && i == strl-e_offset-1 && depth <= 0)) {
                if (on_target == 1)
                    return current_str;

                strcpy(current_str, "");
                current_str_i = 0;
            }

            else if (c != ' ') {
                current_str[current_str_i] = c;
                current_str[current_str_i+1] = '\0';
                current_str_i++;
            }

        } else {
            current_str[current_str_i] = c;
            current_str[current_str_i+1] = '\0';
            current_str_i++;
        }


    }
    s_free(key);
    s_free(current_str);

    return NULL;
}

char* json_parse_arr(char str[], int index) {
    int strl = strlen(str);
    int itemindex = 0;
    int in_string = 0;
    int depth = 0;
    int s_offset = 0;

    for (int i = 0; i < strl; i++) {
        char c = str[i];
        if (c == ' ' || c == '\n' || c == '\t') {
            s_offset++;
        } else
            break;
    }

    char* current_item = s_malloc(sizeof(char) * (strlen(str)+5));
    strcpy(current_item, "");
    for (int i = s_offset+1; i < strl-1; i++) {
        char c = str[i];

        if (c == '"')
            in_string = !in_string;

        if (!in_string) {
            if (c == ' ' || c == '\t' || c == '\n')
                continue;

            else if (c == '[' || c == '{')
                depth++;

            else if (c == '}' || (c == ']' && depth != 0))
                depth--;


            if ((c == ',' && depth == 0)) {
                if (itemindex == index)
                    return current_item;

                strcpy(current_item, "");
                itemindex++;
            }
            else
                strcat(current_item, CHAR_TO_STR(c));

        } else
            strcat(current_item, CHAR_TO_STR(c));

    }

    if (itemindex == index)
        return current_item;

    s_free(str);
    s_free(current_item);
    return NULL;
}
