#include <stdio.h>
#include "requests.h"
#include "simple-json/simple_json.h"

int main() {
    char* x = get("https://aur.archlinux.org/rpc/?v=5&type=info&arg[]=ungoogled-chromium");
    printf("%s\n", x);
}