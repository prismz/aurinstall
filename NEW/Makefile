# MAKE BETTER

CC = cc
CFLAGS := -Wall -Wextra -pedantic 
PRODCFLAGS := -O3
DEBUGCFLAGS := -g
DESTDIR := /usr/local/bin/

CFLAGS += $(shell pkg-config --cflags json-c)
LDFLAGS += $(shell pkg-config --libs json-c)

LDFLAGS += $(shell pkg-config --libs libcurl)
CFLAGS += $(shell pkg-config --cflags libcurl)

all:opts prod

prod:
	$(CC) *.c $(PRODCFLAGS) $(LDFLAGS) $(CFLAGS) -o aurinstall

debug:opts
	$(CC) *.c $(DEBUGCFLAGS) $(LDFLAGS) $(CFLAGS) -o aurinstall

install:prod
	cp -r aurinstall $(DESTDIR)
	mkdir -p /var/cache/aurinstall
	cp default-config.json /etc/aurinstall-config.json

opts:
	@echo CC: $(CC)
	@echo CFLAGS: $(CFLAGS)
	@echo LDFLAGS: $(LDFLAGS)

run:all
	./aurinstall
