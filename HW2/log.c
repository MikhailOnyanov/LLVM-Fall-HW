#include <stdio.h>
#include <stdlib.h>

static FILE *instruction_file = NULL;
static FILE *use_file = NULL;

static void enable_buffering(FILE *f) {
  static char buf[2][1 << 16];
  static int idx = 0;
  setvbuf(f, buf[idx], _IOFBF, sizeof(buf[0]));
  idx = (idx + 1) % 2;
}

static FILE *open_or_die(const char *path) {
  FILE *f = fopen(path, "w");
  if (!f) {
    perror(path);
    exit(1);
  }
  enable_buffering(f);
  return f;
}

FILE *get_instruction_file(void) {
  if (!instruction_file)
    instruction_file = open_or_die("ir_trace.log");
  return instruction_file;
}

FILE *get_use_file(void) {
  if (!use_file)
    use_file = open_or_die("ir_use.log");
  return use_file;
}

__attribute__((destructor)) static void close_logs(void) {
  if (instruction_file)
    fclose(instruction_file);
  if (use_file)
    fclose(use_file);
}
