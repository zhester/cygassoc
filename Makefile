##############################################################################
#
#  Makefile
#
#  Builds the cygassoc program using 64-bit MinGW in a Cygwin environment.
#
#  So far, only used with GNU make under Cygwin.
#
##############################################################################

# Customized build values (defines IMAGE_NAME and WINDOWS_RESOURCE)
include vimassoc.Makefile

# Basic compile environment settings
BINPF   := /usr/bin
CC      := $(BINPF)/i686-w64-mingw32-gcc.exe
CFLAGS   = -Wall -static -mwindows -DWIN32_LEAN_AND_MEAN
LD      := $(CC)
LDFLAGS  = -Wall -static -mwindows -s
WR      := $(BINPF)/i686-w64-mingw32-windres.exe
WRFLAGS := -O coff
SHELL   := $(BINPF)/sh

# Add debug symbols for any debug target.
debug: CFLAGS += -ggdb
debug: LDFLAGS += -ggdb

# Build directory
BLDDIR = build

# Project source files
SOURCES := $(wildcard *.c)

# Derive object targets from source files
OBJECTS := $(patsubst %.c, $(BLDDIR)/%.o, $(SOURCES))

# Windows program resource information
RESOURCE_SOURCE := $(WINDOWS_RESOURCE)
RESOURCE        := $(BLDDIR)/out.res

# Default target
all: $(BLDDIR)/$(IMAGE_NAME)

# How to build the project binary
$(BLDDIR)/$(IMAGE_NAME): $(OBJECTS) $(RESOURCE)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(RESOURCE) && chmod 700 $@

# How to build the resource information
$(RESOURCE): $(RESOURCE_SOURCE) | $(BLDDIR)
	$(WR) $(WRFLAGS) -o $@ $<

# How to build the project's object files
$(BLDDIR)/%.o: %.c *.h | $(BLDDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

# Make sure there's an output directory.
$(BLDDIR):
	mkdir -p $(BLDDIR)

# How to clean the output files
clean:
	rm -rf $(BLDDIR)

