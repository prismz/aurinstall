#include <stdio.h>
#include "requests.h"
#include "simple-json/simple_json.h"
#include "globals.h"
#include "package.h"

int main() {
    Options opts;
    init(&opts);
    char* url = "https://aur.archlinux.org/rpc/?v=5&type=search&arg=ungoogled-chromium";
    struct curl_res_string x = get(url);
    char* result = json_parse_arr(json_parse_dict(x.ptr, "results"), 1);
    PackageData p = parse_package_json(result);
    // print_package_data(p, &opts);
    free_package_data(p);

    char args[2][MAX_ARGLEN];
    strcpy(args[0], "gtk");
    strcpy(args[1], "3");

    search_package("bin", args, 2, &opts);
    
}