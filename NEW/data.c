#include "data.h"
#include "string.h"

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

int gunzip(const char *path)
{
        if (path == NULL)
                return 1;

        char *new_path = path_remove_extension(path);
        printf("gunzip %s to %s\n", path, new_path);

        FILE *fp = fopen(new_path, "wb+");
        if (!fp)
                return 1;

        gzFile file = gzopen(path, "r");
        if (!file)
                return 1;

        for (;;) {
                unsigned char buff[2048];
                int nread = gzread(file, buff, 2047);
                fwrite(buff, nread, 1, fp);
                buff[nread] = 0;

                if (nread > 2047)
                        continue;

                int err;
                const char *err_str = gzerror(file, &err);
                if (err) {
                        fprintf(stderr, "Error: %s\n", err_str);
                        return 1;
                }
                if (gzeof(file))
                        break;
        }

        gzclose(file);
        fclose(fp);
        free(new_path);

        return 0;
}
