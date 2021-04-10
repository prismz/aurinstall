#include "requests.h"
#include "package.h"

int main() {
    // struct curl_res_string res = get("https://aur.archlinux.org/rpc/?v=5&type=search&arg=ungoogled-chromium");
    // printf("%s\n", res.ptr);
    install_package("ungoogled-chromium");
    

}