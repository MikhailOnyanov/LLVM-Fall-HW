#include "sim.h"
#include "grid.h"
#include <stdio.h>


#define GENERATIONS 2000

// Заполняет поле нулями
void initEmptyField(int field[], int size)
{
    for (int i = 0; i < size * size; i++)
    {
        field[i] = 0;
    }
}

// Получает значение клетки с учётом "заворачивания" по краям
static int getCellValue(int field[], int x, int y, int size)
{
    x = (x + size) % size;
    y = (y + size) % size;
    return field[x * size + y];
}

// Считает количество живых соседей клетки
int countNeighbours(int pos_x, int pos_y, int field[], int size)
{
    int count_neighbours = 0;

    for (int dx = -1; dx < 2; dx++)
    {
        int cur_x = pos_x + dx;

        for (int dy = -1; dy < 2; dy++)
        {
            int cur_y = pos_y + dy;

            // Если текущая клетка - пропускаем
            if (pos_x == cur_x && pos_y == cur_y)
                continue;

            if (getCellValue(field, cur_x, cur_y, size) == 1)
                count_neighbours += 1;
        }
    }
    return count_neighbours;
}

// Один шаг симуляции "Жизни"
void stepLife(const int size, int field[], int nextField[])
{
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {

            const int n = countNeighbours(x, y, field, size);

            const int idx = x * size + y;
            if (field[idx] == 1) {
                nextField[idx] = (n == 2 || n == 3) ? 1 : 0;
            } else {
                nextField[idx] = (n == 3) ? 1 : 0;
            }
        }
    }
}

// Инициализировать поле случайным образом: примерно 1/5 клеток живые.
void randomizeField(int field[], int size)
{
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            int alive = (simRand() % 5 == 0) ? 1 : 0;
            field[x * size + y] = alive;
        }
    }
}


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
