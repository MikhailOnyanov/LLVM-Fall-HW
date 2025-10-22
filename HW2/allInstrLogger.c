#include <stdio.h>

// Лог всех инструкций (кроме PHI, Call, Binary и Return)
// funcName  - имя функции, где находится инструкция
// instrName - название инструкции (opcode)
// valID     - уникальный идентификатор инструкции (адрес в памяти)
void allInstrLogger(char *funcName, char *instrName, long int valID) {
    printf("[LOG] In function '%s': instruction '%s' {%ld}\n",
           funcName, instrName, valID);
}