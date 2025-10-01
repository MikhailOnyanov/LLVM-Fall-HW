#ifndef LOGIC_H
#define LOGIC_H

void initEmptyField(int field[], int size);
int  countNeighbours(int pos_x, int pos_y, int field[], int size);

void stepLife(const int size, int field[], int nextField[]);
void randomizeField(int field[], int size);

#endif
