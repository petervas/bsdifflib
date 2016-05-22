CC			=	gcc
CFLAGS		+=	-O2 -Wall -ansi
LDFLAGS		+=	-lbz2

all:		bsdiff bspatch

bsdiff:		bsdiff.o bsdifflib.o
	$(CC) $(LDFLAGS) bsdiff.o bsdifflib.o -o $@

bspatch:	bspatch.o bspatchlib.o
	$(CC) $(LDFLAGS) bspatch.o bspatchlib.o -o $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f bsdiff bspatch bsdiff.o bsdifflib.o bspatch.o bspatchlib.o
