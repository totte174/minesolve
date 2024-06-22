CC := gcc
SRCDIR := src
BUILDDIR := build
BIN := bin
TARGET := $(BIN)/run
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -Wall -O3
DEBUGFLAGS := -Wall -g
INC := -I include -lm

$(shell if ! [ -e $(BIN) ]; then\
	mkdir $(BIN);\
fi;)

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB) -O3

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo " Building..."
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $< -save-temps -O3

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

mssolve:
	$(CC) $(CFLAGS) $(SOURCES) $(INC) -o bin/mssolve

debug:
	$(CC) $(DEBUGFLAGS) $(SOURCES) $(INC) -o bin/mssolve-debug

.PHONY: clean