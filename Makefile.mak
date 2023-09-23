# NMake makefile

# Compiler
CC = cl

# Compiler flags
CFLAGS = /nologo /O2 /W3 /Ibzip2 /D_WIN32 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /D_CRT_SECURE_NO_WARNINGS

# Linker
LINK = link

# Linker flags
LFLAGS = /nologo /MANIFEST

# Source files
BSDIFF_SRCS = bsdiff.c bsdifflib.c
BSPATCH_SRCS = bspatch.c bspatchlib.c
BZIP2_SRCS = bzip2\blocksort.c bzip2\bzlib.c bzip2\compress.c bzip2\crctable.c bzip2\decompress.c bzip2\huffman.c bzip2\randtable.c

# Object files
BSDIFF_OBJS = $(BSDIFF_SRCS:.c=.obj)
BSPATCH_OBJS = $(BSPATCH_SRCS:.c=.obj)
BZIP2_OBJS = $(BZIP2_SRCS:.c=.obj)

# Target executables
BSDIFF_TARGET = bsdiff.exe
BSPATCH_TARGET = bspatch.exe

# Default target
all: $(BSDIFF_TARGET) $(BSPATCH_TARGET)

# Compile C source files
.c.obj:
	$(CC) $(CFLAGS) /c /Fo$(@D)\ $<

# Link object files
$(BSDIFF_TARGET): $(BSDIFF_OBJS) $(BZIP2_OBJS)
	$(LINK) $(LFLAGS) -out:$@ $(BSDIFF_OBJS) $(BZIP2_OBJS)

$(BSPATCH_TARGET): $(BSPATCH_OBJS) $(BZIP2_OBJS)
	$(LINK) $(LFLAGS) -out:$@ $(BSPATCH_OBJS) $(BZIP2_OBJS)

# Clean target
clean:
	del /Q /F $(BSDIFF_OBJS) $(BSPATCH_OBJS) $(BZIP2_OBJS) $(BSDIFF_TARGET) $(BSPATCH_TARGET) $(BSDIFF_TARGET).manifest $(BSPATCH_TARGET).manifest
