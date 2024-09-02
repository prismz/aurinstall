#ifndef SRCINFO_H
#define SRCINFO_H

#include "string.h"

#include <stdio.h>

struct srcinfo {
        char *pkgbase;
        char *pkgname;
        char *pkgver;

        struct stringlist *keylist;
        struct stringlist *depends;
        struct stringlist *makedepends;
};

struct srcinfo *parse_pkg_srcinfo(const char *pkg_name);
void srcinfo_print(struct srcinfo *srcinfo);

#endif  /* SRCINFO_H */
