#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

size_t write_function(void* ptr, size_t size, size_t nmemb, void* userdata) {
    strcat(userdata, ptr);
    return size * nmemb;
}

void* get(char* url) {
    char response_string[4096];
    strcpy(response_string, "");
    char* o = malloc(sizeof(char)*4099);

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        char header_string[4096];
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        
        char* url;
        long response_code;
        double elapsed;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
        
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl = NULL;
    }
    strcpy(o, response_string);
    return o;
}