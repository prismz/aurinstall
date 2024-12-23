# MAKE BETTER

CC = cc
CFLAGS := -Wall -Wextra -pedantic
PRODCFLAGS := -O3
DEBUGCFLAGS := -g
DESTDIR := /usr/local/bin/

CFLAGS += $(shell pkg-config --cflags json-c)
LDFLAGS += $(shell pkg-config --libs json-c)

CFLAGS += $(shell pkg-config --cflags libcurl)
LDFLAGS += $(shell pkg-config --libs libcurl)

CFLAGS += $(shell pkg-config --cflags zlib)
LDFLAGS += $(shell pkg-config --libs zlib)

CFLAGS += $(shell pkg-config --cflags libalpm)
LDFLAGS += $(shell pkg-config --libs libalpm)

# Didn't work with pkg-config?
LDFLAGS += -lpacutils

all:opts prod

prod:opts
	$(CC) *.c util/*.c $(PRODCFLAGS) $(LDFLAGS) $(CFLAGS) -o aurinstall

debug:opts
	$(CC) *.c util/*.c $(DEBUGCFLAGS) $(LDFLAGS) $(CFLAGS) -o aurinstall

install:prod
	cp -r aurinstall $(DESTDIR)

opts:
	@echo CC: $(CC)
	@echo CFLAGS: $(CFLAGS)
	@echo LDFLAGS: $(LDFLAGS)

run:all
	./aurinstall
