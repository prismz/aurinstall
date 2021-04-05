// testing file, for yknow, testing


#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int get_terminal_width() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
}

int main() {
    int x = get_terminal_width();
    int y = get_terminal_width();
    int z = get_terminal_width();
    
    printf("%d %d %d\n", x, y, z);
}