LLVM_CONFIG ?= llvm-config
OPT_LEVEL ?= 2

SDL_INC ?= /opt/homebrew/Cellar/sdl2/2.32.10/include/SDL2
SDL_LIB ?= /opt/homebrew/Cellar/sdl2/2.32.10/lib

CFLAGS := -O$(OPT_LEVEL)
PLUGIN := libAppIRTrace.so
APP_OBJ := app.o
BIN := app_trace

.PHONY: all run clean plugin app link

all: $(BIN)

$(PLUGIN): AppIRTracePass.cpp
	clang++ -fPIC -shared $< -o $@ `$(LLVM_CONFIG) --cxxflags --ldflags --system-libs --libs core passes irreader`

$(APP_OBJ): ../HW1/app.c $(PLUGIN)
	clang ../HW1/app.c -c -fpass-plugin=./$(PLUGIN) $(CFLAGS) -o $@

$(BIN): ../HW1/start.c ../HW1/sim.c ../HW1/grid.c log.c allInstrLogger.c $(APP_OBJ)
	clang $^ -I$(SDL_INC) -L$(SDL_LIB) -lSDL2 $(CFLAGS) -o $@

run: $(BIN)
	./$(BIN)

plugin: $(PLUGIN)
app: $(APP_OBJ)
link: $(BIN)

clean:
	rm -f $(PLUGIN) $(APP_OBJ) $(BIN)
