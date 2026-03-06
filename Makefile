BUILD_DIR  := build
CMAKE      := cmake
BUILD_TYPE ?= Release

WASM_DIR  := build-wasm
EMCC      ?= emcc

WASM_SOURCES := \
	src/lib/board.c \
	src/lib/solver.c \
	src/lib/equations.c \
	src/lib/permutations.c \
	src/lib/probability.c \
	src/lib/minesolve.c

WASM_EXPORTS := ["_ms_board_init","_ms_board_parse","_ms_solve","_ms_probabilities","_malloc","_free"]
WASM_RUNTIME := ["ccall","cwrap","getValue","setValue"]

.PHONY: all debug lib clean install wasm

all: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR)

# Re-configure if CMakeLists.txt changes
$(BUILD_DIR)/Makefile: CMakeLists.txt
	$(CMAKE) -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

debug:
	$(CMAKE) -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	$(MAKE) -C $(BUILD_DIR)

lib: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) minesolve_static minesolve_shared

install: all
	$(MAKE) -C $(BUILD_DIR) install

wasm: $(WASM_DIR)/minesolve.js

$(WASM_DIR)/minesolve.js: $(WASM_SOURCES) $(wildcard include/*.h) | $(WASM_DIR)
	$(EMCC) -O3 -Iinclude $(WASM_SOURCES) -lm \
		-s EXPORTED_FUNCTIONS='$(WASM_EXPORTS)' \
		-s EXPORTED_RUNTIME_METHODS='$(WASM_RUNTIME)' \
		-s MODULARIZE=1 -s EXPORT_NAME=MinesolveModule \
		-s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=33554432 \
		-o $@

$(WASM_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIR) $(WASM_DIR)
