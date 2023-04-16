/*
 * This file is part of aurinstall.
 *
 * aurinstall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * aurinstall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurinstall.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * Copyright (C) 2023 Hasan Zahra
 * https://github.com/prismz/aurinstall
 */

#include "requests.h"
#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

void init_string(struct curl_str *s)
{
        s->len = 0;
        s->ptr = safe_malloc(sizeof(char) * (s->len + 1));
        s->ptr[0] = '\0';
}

size_t write_func(void *ptr, size_t size, size_t nmemb, struct curl_str *s)
{
        size_t new_len = s->len + size * nmemb;
        s->ptr = safe_realloc(s->ptr, new_len + 1);

        memcpy(s->ptr + s->len, ptr, size * nmemb);
        s->ptr[new_len] = '\0';
        s->len = new_len;

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
