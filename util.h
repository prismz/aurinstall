#ifndef UTIL_H
#define UTIL_H

#define BOLD "\033[1m"
#define GREEN "\033[92m"
#define RED "\033[91m"
#define ENDC "\033[0m"

#include <stdio.h>

void die(FILE* buff, char* str, int ret);
void pretty_print(char* str_, int indent);
int dir_is_empty(char* path);
char* get_homedir(void);
void right_pad_print_str(char* str, int max_len, int extra);

#endif
