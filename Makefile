# Compiler and Directories
CC := gcc
SRCDIR := src
BINDIR := bin
TARGET := $(BINDIR)/mssolve
DEBUGTARGET := $(BINDIR)/mssolve-debug
SRCEXT := c
SOURCES := $(wildcard $(SRCDIR)/*.$(SRCEXT))

# Flags and Includes
CFLAGS := -Wall -O3 -pg
DEBUGFLAGS := -Wall -g
INC := -I include
LIBS := -lm

all: mssolve debug

profiler:
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(SOURCES) $(INC) -o $(TARGET) $(LIBS) -pg

mssolve:
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(SOURCES) $(INC) -o $(TARGET) $(LIBS)

clean:
	@echo "Cleaning up..."
	$(RM) -r $(BINDIR)

debug:
	@mkdir -p $(BINDIR)
	$(CC) $(DEBUGFLAGS) $(SOURCES) $(INC) -o $(DEBUGTARGET) $(LIBS)

# Phony targets
.PHONY: clean debug 
