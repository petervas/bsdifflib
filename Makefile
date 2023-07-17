CC          = gcc
CFLAGS      += -O2 -Wall -ansi
LDFLAGS     += -lbz2

OBJECTS     = bsdiff.o bsdifflib.o bspatch.o bspatchlib.o

.PHONY: all clean

all: bsdiff bspatch

bsdiff: bsdiff.o bsdifflib.o
	$(CC) $^ $(LDFLAGS) -o $@

bspatch: bspatch.o bspatchlib.o
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

bsdiff.o: bsdiff.c
bsdifflib.o: bsdifflib.c
bspatch.o: bspatch.c
bspatchlib.o: bspatchlib.c

clean:
	rm -f bsdiff bspatch $(OBJECTS)
