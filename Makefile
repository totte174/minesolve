# Compiler and Directories
CC := gcc
SRCDIR := src
BINDIR := bin
DOCDIR := doc
TARGET := $(BINDIR)/minesolve
DEBUGTARGET := $(BINDIR)/minesolve-debug
SRCEXT := c
SOURCES := $(wildcard $(SRCDIR)/*.$(SRCEXT))

# Flags and Includes
CFLAGS := -Wall -O3 -pg
DEBUGFLAGS := -Wall -g
INC := -I include
LIBS := -lm

all: minesolve debug

docs:
	@mkdir -p $(DOCDIR)
	doxygen Doxyfile

profiler:
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(SOURCES) $(INC) -o $(TARGET) $(LIBS) -pg

minesolve:
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(SOURCES) $(INC) -o $(TARGET) $(LIBS)

clean:
	@echo "Cleaning up..."
	$(RM) -r $(BINDIR) $(DOCDIR)

debug:
	@mkdir -p $(BINDIR)
	$(CC) $(DEBUGFLAGS) $(SOURCES) $(INC) -o $(DEBUGTARGET) $(LIBS)

# Phony targets
.PHONY: clean debug 
