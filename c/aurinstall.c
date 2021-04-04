#include <stdio.h>
#include "requests.h"
#include "simple-json/simple_json.h"
#include "globals.h"
#include "package.h"

int main() {
    char* url = "https://aur.archlinux.org/rpc/?v=5&type=search&arg=ungoogled-chromium";
    struct curl_res_string x = get(url);
    char* result = json_parse_arr(json_parse_dict(x.ptr, "results"), 1);
    // printf("%s\n", resultcount);
    PackageData p = parse_package_json(result);
    // printf("%s", p.desc);
    print_package_data(p);
    
}