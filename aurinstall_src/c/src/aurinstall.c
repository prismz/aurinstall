#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

#include "requests.h"
#include "package.h"
#include "util.h"

int main() {
    init();
    // struct curl_res_string res = get("https://aur.archlinux.org/rpc/?v=5&type=search&arg=ungoogled-chromium");
    // printf("%s\n", res.ptr);
    // install_package("ungoogled-chromium");
    // char searchterms[SEARCH_MAX_ARG_COUNT][SEARCH_MAX_ARG_LEN];
    // strcpy(searchterms[0],"ungoogled-chromium");
    // strcpy(search_package)
    // search_package(searchterms, 1);
    // printf("%d\n", is_confirmation("y"));
    // system("git clone https://aur.archlinux.org/ungoogled-chromium.git ~/.cache/aurinstall/ungoogled-chromium");
    // printf("%s\n", getenv("HOME"));
    cleanup();
}