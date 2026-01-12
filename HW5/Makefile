.PHONY: all run exec clean test

CXX = clang++
FLEX = flex
BISON = bison
LLVM_CONFIG ?= llvm-config
LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags)
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags --libs core executionengine interpreter mcjit native)
CXXFLAGS = -std=c++17 $(LLVM_CXXFLAGS) -Wno-deprecated-register

SIM_DIR = ../game-of-live-c/sim_app
SDL_FLAGS = -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -Wl,-rpath,/opt/homebrew/lib

all: run

PixelFlow.tab.c PixelFlow.tab.h: PixelFlow.y
	$(BISON) -d PixelFlow.y

lex.yy.c: PixelFlow.lex PixelFlow.tab.h
	$(FLEX) PixelFlow.lex

pixelflow: lex.yy.c PixelFlow.tab.c
	$(CXX) $(CXXFLAGS) -o pixelflow PixelFlow.tab.c lex.yy.c $(LLVM_LDFLAGS)

pixelflow_exec: lex.yy.c PixelFlow.tab.c $(SIM_DIR)/sim.c
	clang -c $(SIM_DIR)/sim.c $(SDL_FLAGS) -o sim.o
	$(CXX) $(CXXFLAGS) -DWITH_EXECUTION_ENGINE -o pixelflow_exec PixelFlow.tab.c lex.yy.c sim.o $(LLVM_LDFLAGS) $(SDL_FLAGS)

game_of_life.ll: pixelflow game_of_life.pflow
	./pixelflow < game_of_life.pflow > game_of_life.ll

game_of_life: game_of_life.ll
	clang game_of_life.ll $(SIM_DIR)/sim.c $(SIM_DIR)/start.c \
		$(SDL_FLAGS) \
		-o game_of_life

run: game_of_life
	./game_of_life

exec: pixelflow_exec game_of_life.pflow
	@echo "JIT mode with ExecutionEngine"
	./pixelflow_exec --exec < game_of_life.pflow

test: pixelflow
	@./pixelflow < tests/test.pflow > test.ll
	@clang test.ll $(SIM_DIR)/sim.c $(SIM_DIR)/start.c $(SDL_FLAGS) -o test_app
	@./test_app
	@rm -f test.ll test_app

clean:
	rm -f pixelflow pixelflow_exec
	rm -f lex.yy.c
	rm -f PixelFlow.tab.c PixelFlow.tab.h
	rm -f game_of_life.ll game_of_life sim.o
	rm -f *.s *.ll test_app
