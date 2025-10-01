#include "sim.h"
#include "logic.h"
#include "grid.h"

#include <stdio.h>


#define GENERATIONS 2000


void app(void)
{
    const int size = GRID_SIZE;

    // Два буфера для "перекладывания" поколений
    static int field[GRID_SIZE * GRID_SIZE];
    static int nextField[GRID_SIZE * GRID_SIZE];

    initEmptyField(field, size);
    randomizeField(field, size);

    // Главный цикл симуляции
    for (int gen = 0; gen < GENERATIONS; ++gen) {
        renderField(size, field);
        stepLife(size, field, nextField);

        // Обменять буферы: next -> field
        for (int i = 0; i < size * size; ++i)
            field[i] = nextField[i];
    }

    renderField(size, field);
}
