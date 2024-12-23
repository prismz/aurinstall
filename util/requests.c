#include "requests.h"
#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

static void init_string(struct curl_str *s)
{
        s->len = 0;
        s->ptr = safe_malloc(sizeof(char) * (s->len + 1));
        s->ptr[0] = '\0';
}

static size_t write_func(void *ptr, size_t size, size_t nmemb, struct curl_str *s)
{
        size_t new_len = s->len + size * nmemb;
        s->ptr = safe_realloc(s->ptr, new_len + 1);

        memcpy(s->ptr + s->len, ptr, size * nmemb);
        s->ptr[new_len] = '\0';
        s->len = new_len;

        return size * nmemb;
}

static size_t write_tofile_func(void *ptr, size_t size, size_t nmemb, struct curl_file *f)
{
        fwrite(ptr, size, nmemb, f->fp);
        return size * nmemb;
}

char *requests_get(char *url)
{
        struct curl_str s;
        init_string(&s);

        CURL *curl = curl_easy_init();

        if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
                curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10);
                curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1);

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
                curl_easy_perform(curl);

                curl_easy_cleanup(curl);
        } else {
                free(s.ptr);
                return NULL;
        }

        return s.ptr;
}

int download_to_file(const char *url, const char *path)
{
        struct curl_file f;
        FILE *fp = fopen(path, "wb+");
        if (!fp)
                return 5;

        f.fp = fp;
        CURL *curl = curl_easy_init();
        if (!curl)
                return 2;

        printf("downloading %s to %s...\n", url, path);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_tofile_func);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &f);

        curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        fclose(fp);

        return 0;
}
