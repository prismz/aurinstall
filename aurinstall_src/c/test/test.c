#include <stdio.h>
#include <stdlib.h>
#include "header.h"

int main() {
    struct example_struct s;
    // s.value = 93;
    modify_val(&s, 100);
    printf("%d\n", s.value);
}