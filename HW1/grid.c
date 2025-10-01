#include "sim.h"
#include <stdio.h>

// Масштаб: во сколько пикселей рисуем одну клетку.
// SIM_X_SIZE и SIM_Y_SIZE заданы в sim.h (512 x 512). Должны нацело делиться на SCALE.
#define SCALE (SIM_X_SIZE / GRID_SIZE)
#if __STDC_VERSION__ >= 201112L
_Static_assert(GRID_SIZE > 0, "GRID_SIZE must be > 0");
_Static_assert(SIM_X_SIZE % GRID_SIZE == 0, "SIM_X_SIZE must be divisible by GRID_SIZE");
_Static_assert(SIM_Y_SIZE % GRID_SIZE == 0, "SIM_Y_SIZE must be divisible by GRID_SIZE");
#endif

// Поставить залитый квадратик клетки размера SCALE x SCALE
void drawCell(int cx, int cy, int color)
{
    const int x0 = cx * SCALE;
    const int y0 = cy * SCALE;
    for (int dy = 0; dy < SCALE; ++dy)
        for (int dx = 0; dx < SCALE; ++dx)
            simPutPixel(x0 + dx, y0 + dy, color);
}

// Нарисовать фоновую заливку
void clearBackground(void)
{
    for (int y = 0; y < SIM_Y_SIZE; ++y)
        for (int x = 0; x < SIM_X_SIZE; ++x)
            simPutPixel(x, y, COLOR_BG);
}

// (Необязательно) нарисовать линиями сетку, чтобы клетки читались лучше.
void drawGrid(void)
{
    // Вертикальные линии внутри поля
    for (int gx = 0; gx < GRID_SIZE; ++gx) {
        int x = gx * SCALE;                 // 0 .. SIM_X_SIZE - SCALE
        for (int y = 0; y < SIM_Y_SIZE; ++y)
            simPutPixel(x, y, COLOR_GRID);
    }
    // Правая внешняя граница
    for (int y = 0; y < SIM_Y_SIZE; ++y)
        simPutPixel(SIM_X_SIZE - 1, y, COLOR_GRID);

    // Горизонтальные линии внутри поля
    for (int gy = 0; gy < GRID_SIZE; ++gy) {
        int y = gy * SCALE;                 // 0 .. SIM_Y_SIZE - SCALE
        for (int x = 0; x < SIM_X_SIZE; ++x)
            simPutPixel(x, y, COLOR_GRID);
    }
    // Нижняя внешняя граница
    for (int x = 0; x < SIM_X_SIZE; ++x)
        simPutPixel(x, SIM_Y_SIZE - 1, COLOR_GRID);
}

// Отрисовать текущее состояние field
void renderField(const int size, const int field[])
{
    clearBackground();

    // Сами клетки
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (field[x * size + y]) {
                drawCell(x, y, COLOR_ALIVE);
            }
        }
    }

    // Рисуем вспомогательную сетку
    drawGrid();

    // Показать кадр
    simFlush();
}