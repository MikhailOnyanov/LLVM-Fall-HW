#ifndef SIM_H
#define SIM_H

// Цвета RGB (0xRRGGBB)
#define COLOR_BG    0x101010  // фон
#define COLOR_ALIVE 0xE0E0E0  // живая клетка
#define COLOR_GRID  0x202020  // вспомогательная сетка (тонкая)

#define SIM_X_SIZE 512
#define SIM_Y_SIZE 512
#define GRID_SIZE 128

#ifndef __sim__
void simInit();
void app();
void simExit();
void simFlush();
void simPutPixel(int x, int y, int argb);
void initEmptyField(int field[], int size);
int countNeighbours(int pos_x, int pos_y, int field[], int size);
int simRand(void);
#endif // __sim__

#endif // SIM_H