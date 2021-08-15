all:
	cc *.c -lcurl -ljson-c -Wall -Wextra -pedantic -o aurinstall

install:all
	cp aurinstall /usr/local/bin