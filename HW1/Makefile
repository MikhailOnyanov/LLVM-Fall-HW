CC      := clang
CSTD    := -std=c99
CFLAGS  := $(CSTD) -Wall -Wextra -ggdb3 -O0 $(shell sdl2-config --cflags)

# Флаги линковки:
#  sdl2-config --libs — нужные библиотеки/опции линковщика для SDL2
#  -lm                — математическая библиотека
LDFLAGS := $(shell sdl2-config --libs) -lm

# Список всех .c в каталоге
SRCS := $(wildcard *.c)
# Соответствующие им объектные файлы .o
OBJS := $(SRCS:.c=.o)

# Итоговая цель "run": линковка всех .o в исполняемый файл "run"
run: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

# Правило сборки каждого .o из соответствующего .c
#  $< — входной файл (исходник .c)
#  $@ — выходной файл (объект .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# "clean" — служебная цель (не файл). Удаляет артефакты сборки.
.PHONY: clean
clean:
	rm -f $(OBJS) run
