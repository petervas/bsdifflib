CC			=	gcc
CFLAGS		+=	-O2 -Wall -ansi
LDFLAGS		+=	-lbz2

all:		bsdiff bspatch

bsdiff:		bsdiff.o bsdifflib.o
	$(CC) bsdiff.o bsdifflib.o $(LDFLAGS) -o $@

bspatch:	bspatch.o bspatchlib.o
	$(CC) bspatch.o bspatchlib.o $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f bsdiff bspatch bsdiff.o bsdifflib.o bspatch.o bspatchlib.o
