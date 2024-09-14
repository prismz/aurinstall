CC = cc
CFLAGS := -ljson-c -Wall -Wextra -pedantic
PRODCFLAGS := -O3
DEBUGCFLAGS := -g

all:
	$(CC) *.c $(CFLAGS) $(PRODCFLAGS) -o fastdeps

debug:
	$(CC) *.c $(CFLAGS) $(DEBUGCFLAGS) -o fastdeps

run:all
	./fastdeps

drun:debug
	./fastdeps
