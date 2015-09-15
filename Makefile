SHELL = /bin/bash
DEBUGFLAGS = -D__DEBUG_ON__
CFLAGS = -O2 \
         $(DEBUGFLAGS)
TECHNO = $(shell uname -m)
LDFLAGS = 

LIBSOURCES=$(shell echo lib/*.c)
LIBSINCLUDES=-Ilib

OBJECTSDIR=$(TECHNO).objects

OBJECTS=$(addprefix $(OBJECTSDIR)/, $(LIBSOURCES:.c=.o))

$(OBJECTSDIR)/lib/%.o: lib/%.c
	@$(CC) -c $(CFLAGS) $(LIBSINCLUDES) -MM -MT $(TECHNO).objects/lib/$*.o lib/$*.c > .deps/$*.dep
	$(CC) -c $(CFLAGS) $(LIBSINCLUDES) lib/$*.c -o $(TECHNO).objects/lib/$*.o

all: depsdir $(TECHNO).objects/lib liblcd.a lcd_echo

depsdir:
	@mkdir -p .deps

$(OBJECTSDIR)/lib:
	@mkdir -p $(OBJECTSDIR)/lib

liblcd.a: $(OBJECTS)
	ar -rcs liblcd.a $(OBJECTS)

lcd_echo.o: lcd_echo.c
	$(CC) -c $(CFLAGS) $(LIBSINCLUDES) lcd_echo.c -o lcd_echo.o

lcd_echo: liblcd.a lcd_echo.o
	$(CC) lcd_echo.o liblcd.a $(LDFLAGS) -o $@
	strip lcd_echo

clean:
	rm -f $(TECHNO).objects/lib/*.o *.o *.a *.tar lcd_echo .deps/*.dep

-include .deps/*.dep
