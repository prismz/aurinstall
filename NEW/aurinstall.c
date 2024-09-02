#include "options.h"
#include "util/util.h"
#include "util/target.h"
#include "install.h"

int main(void)
{
        if (init())
                fatal_err("failed to initialize");
        printf("Done initializing.\n");

        char *targets[8] = { "osu-lazer-bin", "librewolf", "qtcreator-git", "firefox" };
        install_packages(targets, 4);
}
