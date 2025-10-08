#include <stdio.h>

void traceInstruction(const char* func, const char* inst) {
    printf("[TRACE] %s: %s\n", func, inst);
}
