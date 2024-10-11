#include "options.h"
#include "util/util.h"
#include "util/fastdeps.h"
#include "install.h"

int main(void)
{
        if (init())
                fatal_err("failed to initialize");
        printf("Done initializing.\n");

        const char *targets[8] = { "osu-lazer-bin", "librewolf", "qtcreator-git", "firefox" };
        install_packages(targets, 4);
}
