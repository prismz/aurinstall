#include "print.h"
#include "alloc.h"

#include <stdio.h>
#include <string.h>

/*
 * should only be used by update_from_handle()
 * no newline
 */
void print_version_difference(const char *old, const char *new,
                size_t longest_vers_len)
{
        /*
        int padlen = longest_vers_len - strlen(old);

        char padding[longest_vers_len + 1];
        memset(padding, ' ', longest_vers_len);
        padding[longest_vers_len] = 0;



        printf("%s%*.*s", old, padlen,
                        padlen, padding);
        printf(" ->  ");
        printf("%s", new);
        */

        /* find where they start to differ */
        size_t oldlen = strlen(old);
        size_t newlen = strlen(new);
        size_t max = (oldlen > newlen) ? oldlen : newlen;

        size_t diff_idx = 0;
        for (diff_idx = 0; diff_idx < max; diff_idx++) {
                if (old[diff_idx] != new[diff_idx])
                        break;
        }

        /* print the part of the old version thats the same */
        printf("%s", GREEN);
        for (size_t i = 0; i < diff_idx; i++)
                printf("%c", old[i]);
        printf("%s%s", ENDC, RED);

        /* print the part thats different */
        for (size_t i = diff_idx; i < oldlen; i++)
                printf("%c", old[i]);
        printf("%s", ENDC);

        /* pad with spaces */
        for (size_t i = 0; i < longest_vers_len - oldlen; i++)
                printf(" ");

        printf("%s->  %s", BLUE, ENDC);

        /* print the part of the new version thats the same */
        printf("%s", GREEN);
        for (size_t i = 0; i < diff_idx; i++)
                printf("%c", new[i]);
        printf("%s%s", ENDC, RED);

        /* print the part thats different */
        for (size_t i = diff_idx; i < newlen; i++)
                printf("%c", new[i]);
        printf("%s", ENDC);
}
