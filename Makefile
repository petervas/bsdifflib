CC          = gcc
CFLAGS      += -O2 -Wall
LDFLAGS     += -lbz2

SOURCES     = bsdiff.c bsdifflib.c bspatch.c bspatchlib.c
OBJECTS     = $(SOURCES:.c=.o)
EXECUTABLES = bsdiff bspatch

.PHONY: all clean

all: $(EXECUTABLES)

bsdiff: bsdiff.o bsdifflib.o
bspatch: bspatch.o bspatchlib.o

$(EXECUTABLES): %: %.o
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(EXECUTABLES) $(OBJECTS)
