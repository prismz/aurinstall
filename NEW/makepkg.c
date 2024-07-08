#include "makepkg.h"
#include "alloc.h"
#include "util.h"

#include <stdio.h>

struct srcinfo *parse_srcinfo(const char *path)
{
        FILE *fp = fopen(path, "rb");
        if (!fp)
                return NULL;

        struct srcinfo *info = safe_calloc(1, sizeof(struct srcinfo));
        char buff[4096];
        while (fgets(buff, 4096, fp) != NULL) {
                char *stripped = trim_whitespace(buff);
        }
}
