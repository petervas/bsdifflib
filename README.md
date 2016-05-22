Binary diff/patch library (bsdifflib/bspatchlib) 1.1 merges the original
Binary diff/patch utility (bsdiff/bspatch) and the Win32 port and adds a 
middle layer of code to make it usable as an ANSI C compliant 
cross-platform C/C++ library. See bsdiff.c/bspatch.c for example usage.
Copyright 2010 Peter Vaskovic <petervaskovic@yahoo.de>

based on Binary diff/patch utility version 4.3, written by 
Copyright 2003-2005 Colin Percival <cperciva@freebsd.org>
available at http://www.daemonology.net/bsdiff/

based on bsdiff/bspatch native Win32-Port (May 2007), written by 
Copyright 2007 Andreas John <dynacore@tesla.inka.de>
available at http://sites.inka.de/tesla/others.html#bsdiff

source release includes files from libbzip2 1.0.6 (20 September 2010)
Copyright 1996-2010 Julian R Seward <jseward@bzip.org>
available at http://www.bzip.org/

error handling was inspired by GetWAD, written by
Copyright 2003-2010 Hippocrates Sendoukas <hsendoukas@hotmail.com>
available at http://getwad.keystone.gr/

The source code was formatted with Artistic Style 1.24
available at http://astyle.sourceforge.net/
using the options "-A1txjz1f".

I would like to thank Hippocrates Sendoukas for helping me out with some
C related questions and giving me helpful coding suggestions in general.
You can visit his site at http://hs.keystone.gr/

Changelog:

    bsdifflib/bspatchlib 1.0 (26 May 2010)

        Initial release.

    bsdifflib/bspatchlib 1.1 (9 November 2010)

        Included own header in source files.
        Hippocrates Sendoukas optimized the patching process to 
        take place in memory. New functions have been added to the bspatchlib
        API to make use of the optimizations for certain use cases.
        Updated libbzip2 to 1.0.6.

-------------------------------------------------------------------------
Quick overview from the homepage of these tools:
http://www.daemonology.net/bsdiff/

Binary diff/patch utility
bsdiff and bspatch are tools for building and applying patches to binary
files. By using suffix sorting (specifically, Larsson and Sadakane's
qsufsort) and taking advantage of how executable files change, bsdiff
routinely produces binary patches 50-80% smaller than those produced by
Xdelta, and 15% smaller than those produced by .RTPatch (a $2750/seat
commercial patch tool). 

These programs were originally named bdiff and bpatch, but the large
number of other programs using those names lead to confusion; I'm not
sure if the "bs" in refers to "binary software" (because bsdiff produces
exceptionally small patches for executable files) or "bytewise
subtraction" (which is the key to how well it performs). Feel free to
offer other suggestions. 

bsdiff and bspatch use bzip2; by default they assume it is in /usr/bin. 

bsdiff is quite memory-hungry. It requires max(17*n,9*n+m)+O(1) bytes of
memory, where n is the size of the old file and m is the size of the new
file. bspatch requires n+m+O(1) bytes. 

bsdiff runs in O((n+m) log n) time; on a 200MHz Pentium Pro, building a
binary patch for a 4MB file takes about 90 seconds. bspatch runs in
O(n+m) time; on the same machine, applying that patch takes about two
seconds. 

Providing that off_t is defined properly, bsdiff and bspatch support
files of up to 2^61-1 = 2Ei-1 bytes.
-------------------------------------------------------------------------
