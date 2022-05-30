include config.mk

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

all: options batquery

options:
	@echo batquery build options:
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "CC      = $(CC)"

.c.o:
	$(CC) -c $(CFLAGS) $<

batquery: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f batquery $(OBJ)

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f batquery $(DESTDIR)$(PREFIX)/bin
	chmod 775 $(DESTDIR)$(PREFIX)/bin/batquery

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/batquery

.PHONY: all options clean install uninstall
