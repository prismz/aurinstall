#include "aurinstall.h"
#include "depend.h"
#include "repo.h"
#include "install.h"
#include "util.h"
#include "depend.h"
#include "options.h"

#include "new.h"

#include <alpm.h>
#include <stdio.h>
#include <stdbool.h>

int main(void)
{
        if (init())
                fatal_err("failed to initialize");

        const char *targets[] = { "qtcreator-git", "librewolf", "osu-lazer-bin", NULL };
        install_packages(targets, 3);
}
