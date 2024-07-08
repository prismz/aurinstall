#ifndef REQUESTS_H
#define REQUESTS_H

#include <stdio.h>
#include <stdlib.h>

struct curl_str {
        char *ptr;
        size_t len;
};

struct curl_file {
        FILE *fp;
};

char *requests_get(char *url);
int download_to_file(const char *url, const char *path);

#endif  /* REQUESTS_H */
