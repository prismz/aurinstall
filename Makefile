all:
	cc -g *.c -lcurl -Wall -Wextra -pedantic -o aurinstall

install:all
	cp aurinstall /usr/local/bin/aurinstall

clean:
	rm aurinstall

uninstall:
	rm /usr/local/bin/aurinstall
