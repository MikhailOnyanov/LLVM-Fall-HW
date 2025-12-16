LLVM_CONFIG ?= llvm-config
SDL_INC ?= /opt/homebrew/Cellar/sdl2/2.32.10/include/SDL2
SDL_LIB ?= /opt/homebrew/Cellar/sdl2/2.32.10/lib
LIBCXX ?= c++
LLVM_LIBDIR := $(shell $(LLVM_CONFIG) --libdir)
LLVM_LIBCXXDIR := $(LLVM_LIBDIR)/c++

LLVM_CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags)
LLVM_LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags --system-libs --libs core executionengine native mcjit)

CXXFLAGS := -std=c++17 -O2 $(LLVM_CXXFLAGS) -I../HW1
LDFLAGS := $(LLVM_LDFLAGS) -L$(SDL_LIB) -lSDL2 \
	-L$(LLVM_LIBDIR) -Wl,-rpath,$(LLVM_LIBDIR) \
	-L$(LLVM_LIBCXXDIR) -Wl,-rpath,$(LLVM_LIBCXXDIR) \
	-l$(LIBCXX) -lc++abi

ASM ?= app_game_of_life.s
BIN_EMUL := asm2ir_emul
BIN_FULL := asm2ir_full

.PHONY: all clean run_emul run_full

all: $(BIN_EMUL) $(BIN_FULL)

$(BIN_EMUL): asm2ir_emul.cpp ../HW1/sim.c
	clang++ $^ $(CXXFLAGS) -I$(SDL_INC) $(LDFLAGS) -o $@

$(BIN_FULL): asm2ir_full.cpp ../HW1/sim.c
	clang++ $^ $(CXXFLAGS) -I$(SDL_INC) $(LDFLAGS) -o $@

run_emul: $(BIN_EMUL)
	./$(BIN_EMUL) $(ASM)

run_full: $(BIN_FULL)
	./$(BIN_FULL) $(ASM)

clean:
	rm -f $(BIN_EMUL) $(BIN_FULL)
