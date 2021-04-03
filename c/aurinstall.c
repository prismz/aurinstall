#include <stdio.h>
#include "requests.h"
#include "simple-json/simple_json.h"

int main() {
    char* url = "https://aur.archlinux.org/rpc/?v=5&type=search&arg=gtk";
    struct string x = get(url);
    printf("beginning JSON parse...\n");
    // char* resultcount = json_parse_dict(x.ptr, "resultcount");
    // printf("%s\n", strip_str(x.ptr));
    strip_str(x.ptr);
}