# Compiler and Directories
CC := gcc
SRCDIR := src
BUILDDIR := build
BINDIR := bin
TARGET := $(BINDIR)/mssolve
SRCEXT := c

# Source and Object Files
SOURCES := $(wildcard $(SRCDIR)/*.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

# Flags and Includes
CFLAGS := -Wall -O3
DEBUGFLAGS := -Wall -g
INC := -I include
LIBS := -lm

# Linking target
$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	@echo "Linking $@..."
	$(CC) $(OBJECTS) -o $@ $(LIBS)

# Compile source files into object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INC) -c -o $@ $< -MMD -MF $(@:.o=.d)

# Include dependency files
DEPS := $(OBJECTS:.o=.d)
-include $(DEPS)

# Clean build and binary files
clean:
	@echo "Cleaning up..."
	$(RM) -r $(BUILDDIR) $(BINDIR)

# Phony targets
.PHONY: clean
