#include <stdio.h>

FILE *get_instruction_file(void);
FILE *get_use_file(void);

void log_instruction(const char *opcode) {
  FILE *f = get_instruction_file();
  fprintf(f, "%s\n", opcode);
}

void log_use(const char *user, const char *operand) {
  FILE *f = get_use_file();
  fprintf(f, "%s <- %s\n", user, operand);
}
