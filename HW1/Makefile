CC      := clang
CSTD    := -std=c99
CFLAGS  := $(CSTD) -Wall -Wextra -ggdb3 -O0 $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs) -lm

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

run: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) run
