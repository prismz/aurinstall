#include "requests.h"
#include "mem.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
init_string(struct curl_str *s)
{
    s->len = 0;
    s->ptr = smalloc(s->len+1, "s->ptr - init_string() - requests.c");
    s->ptr[0] = '\0';
}

size_t
write_func(void *ptr, size_t size, size_t nmemb, struct curl_str *s)
{
    size_t new_len = s->len + size*nmemb;
    s->ptr = srealloc(s->ptr, new_len+1);
    
    memcpy(s->ptr+s->len, ptr, size*nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size*nmemb;
}

struct curl_str
requests_get(char* url)
{
    struct curl_str s;
    init_string(&s);

    CURL* curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        curl_easy_perform(curl);

        curl_easy_cleanup(curl);
    }
    
    return s;
}