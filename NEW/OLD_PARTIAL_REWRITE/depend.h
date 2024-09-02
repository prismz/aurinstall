#ifndef DEPEND_H
#define DEPEND_H

#include "options.h"
#include "srcinfo.h"
#include "string.h"

#include <stdio.h>

int parse_srcinfo_deps(struct srcinfo *srcinfo,
                struct stringlist **repo_deps, struct stringlist **repo_bdeps,
                struct stringlist **aur_deps, struct stringlist **aur_bdeps,
                struct stringlist **pgp_keys);

#endif  /* DEPEND_H */
