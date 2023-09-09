Binary diff/patch library (bsdifflib/bspatchlib) 1.2 is based on the original
binary diff/patch utility (bsdiff/bspatch) by Colin Percival and the Win32
port by Andreas John.

Binary diff/patch library adds an API to make it usable as a cross-platform
C/C++ library. This library generates patches that are compatible with the
original bsdiff tool. The patch routine now works on memory instead of files
so multiple patches in succession can be applied without disk access.

See bsdiff.c/bspatch.c for example usage.

Error handling was inspired by GetWAD, written by
Copyright 2003-2010 Hippocrates Sendoukas <hsendoukas@hotmail.com>
available at http://getwad.keystone.gr/

I would like to thank Hippocrates Sendoukas for helping me out with some
C related questions and giving me helpful coding suggestions in general.
You can visit his site at http://hs.keystone.gr/

Compile:

    Linux: Just run make
    Windows: Unpack bzip2 source code to the bzip2 folder.
             Run nmake -f Makefile.mak

Changelog:

    bsdifflib/bspatchlib 1.2 (9 September 2023)

        Removed Visual Studio Project files.
        Removed bzip2 source code.
        Added nmake file for Windows.
        Updated Makefile.
        Minor code cleanups.
        Pass patch file only to bspatch to print decompressed ctrl/diff/extra sizes.
        Fixed CVE-2014-9862.
        Fixed CVE-2020-14315.
        Fixed memory leaks and add more checks in decompress_block.
        Fixed handling of empty source/destination files.

    bsdifflib/bspatchlib 1.1 (9 November 2010)

        Included own header in source files.
        Hippocrates Sendoukas optimized the patching process to
        take place in memory. New functions have been added to the bspatchlib
        API to make use of the optimizations for certain use cases.
        Updated libbzip2 to 1.0.6.

    bsdifflib/bspatchlib 1.0 (26 May 2010)

        Initial release.
