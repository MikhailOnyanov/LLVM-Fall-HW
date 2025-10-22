# Компилятор и флаги
CXX = clang++
CXXFLAGS = -std=c++14 -O2
LLVM_FLAGS = $(shell llvm-config --cxxflags --ldflags --libs core executionengine native)

# Пути к SDL2
SDL2_INCLUDE = -I/opt/homebrew/Cellar/sdl2/2.32.10/include/SDL2
SDL2_LIB = -L/opt/homebrew/Cellar/sdl2/2.32.10/lib -lSDL2

# Пути к исходным файлам
HW1_DIR = ../HW1
INCLUDE = -I$(HW1_DIR) $(SDL2_INCLUDE)
LIBS = $(SDL2_LIB)

# Исходные файлы - ИСКЛЮЧАЕМ start.c, так как в нем есть app()
IR_SOURCE = game_of_life_ir.cpp
HW1_SOURCES = $(HW1_DIR)/sim.c $(HW1_DIR)/grid.c

# Целевой файл
TARGET = game_of_life_ir

# Правило по умолчанию
all: $(TARGET)

# Компиляция IR генератора
$(TARGET): $(IR_SOURCE) $(HW1_SOURCES)
	$(CXX) $(CXXFLAGS) $(IR_SOURCE) $(HW1_SOURCES) $(INCLUDE) $(LLVM_FLAGS) $(LIBS) -o $(TARGET)

# Очистка
clean:
	rm -f $(TARGET)

# Перекомпиляция
rebuild: clean all

.PHONY: all clean rebuild