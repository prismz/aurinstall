#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>

#define BOLD "\033[1m"
#define RED "\033[91m"
#define GREEN "\033[92m"
#define BLUE "\033[94m"
#define ENDC "\033[0m"

void print_version_difference(const char *old, const char *new,
                size_t longest_vers_len);

#endif  /* PRINT_H */
