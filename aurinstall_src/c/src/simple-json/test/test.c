#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../simple_json.h"

int main() {
    FILE* fp = fopen("o.json", "rb");
    fseek(fp, 0L, SEEK_END);
    int sz = (int)ftell(fp);
    rewind(fp);
    
    char* json_contents = s_malloc(sizeof(char) * (sz+24));
    strcpy(json_contents, "");


    char tmp[sz];
    while (fgets(tmp, sz, fp)) {
        strcat(json_contents, tmp);
    }
    fclose(fp);
    strcpy(tmp, "");


    time_t t = clock();


    char* y = json_parse_dict(json_contents, "results");
    char* x = json_parse_arr(y, 100);


    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC;

    printf("%s\n\n", x);
    s_free(x);
    s_free(json_contents);

    printf("final memory usage: %d bytes\n", mallocated);
    printf("time taken: %fs\n", time_taken);
}
