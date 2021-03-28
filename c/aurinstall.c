#include <stdio.h>
#include "requests.h"
#include "json.h"

int
main() {
    char* x = get("https://aur.archlinux.org/rpc/?v=5&type=info&arg[]=ungoogled-chromium");
    // printf("%s\n", x);

    char* e = "{"
    "\"name\":\"John\","
    "\"age\":18,"
    "\"friends\": [\"Bob\",\"Jake\",\"Daryl\"]"
    "}";

    parse_package_json(x);
}