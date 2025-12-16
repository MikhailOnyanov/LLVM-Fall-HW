# Кастомный ASM

Это кусок отрисовки одного кадра из [app_game_of_life.s](/HW4/app_game_of_life.s):
```asm
...
draw_x_loop:
    GET_CELL x6 x0 x5 x4
    SELECT_COLOR x7 x6 0xFFFFFFFF 0xFF000000
    DRAW_CELL_4x4 x5 x4 x7
    INC_NEi x8 x5 128
    BR_COND x8 draw_x_loop
    INC_NEi x8 x4 128
    BR_COND x8 draw_y_loop
    FLUSH
...
```

## Регистры
- `x0`–`x15`: 32-битные.
- Game of Life: `x0` — текущий буфер, `x1` — следующий буфер, `x2` — счётчик поколений, `x3`–`x9` — временные/координаты.

## Базовые инструкции
- `XOR rd ra rb`: `rd = ra ^ rb`.
- `MUL rd ra rb`: `rd = ra * rb`.
- `SUBi rd ra imm`: `rd = ra - imm`.
- `INC_NEi rd ra imm`: `ra = ra + 1; rd = (ra != imm)`; удобно для циклов «пока != limit».
- `BR_COND rcond label`: если `rcond != 0`, переход на `label`, иначе — падение дальше.
- `BR label`: безусловный переход.
- `PUT_PIXEL rx ry rcolor`: `simPutPixel(rx, ry, rcolor)`.
- `FLUSH`: `simFlush()`.
- `EXIT`: завершить программу.

## Макро для Game of life
- Реализуются либо через `do_*`, либо разворачиваются в IR:
  - `ALLOC_ARRAYS rcur rnxt`: выделить два буфера `GRID_SIZE*GRID_SIZE` `int`, сохранить адреса в `rcur/rnxt`.
  - `CLEAR_ARRAY base len`: занулить `len` элементов `int` от `base`.
  - `COPY_ARRAY dst src len`: скопировать `len` элементов `int`.
  - `GET_CELL dst base x y`: загрузить `base[x * GRID_SIZE + y]` в `dst`.
  - `SET_CELL base x y val`: записать `val` по индексу.
  - `COUNT_NEIGHBORS dst base x y`: подсчёт соседей с заворачиванием по краям.
  - `GAME_OF_LIFE_RULE dst cur neigh`: если `cur == 1`, то `dst = (neigh == 2 || neigh == 3)`, иначе `dst = (neigh == 3)`.
  - `RANDOMIZE_CELL base x y threshold`: поставить 1 с вероятностью `1/threshold` (`simRand() % threshold == 0`).
  - `DRAW_CELL_4x4 x y color`: нарисовать клетку 4x4.
  - `SELECT_COLOR dst cond color_true color_false`: выбрать цвет по флагу.

## Трассировка в IR
- Каждая инструкция (кроме `BR`/меток) может мапиться на `do_*`:
  - `XOR -> call do_XOR(i32 rd, i32 ra, i32 rb)`.
  - `MUL -> call do_MUL(i32 rd, i32 ra, i32 rb)`.
  - `SUBi -> call do_SUBi(i32 rd, i32 ra, i32 imm)`.
  - `INC_NEi -> call do_INC_NEi(i32 rd, i32 ra, i32 imm)`.
  - `PUT_PIXEL -> call do_PUT_PIXEL(i32 rx, i32 ry, i32 rcolor)`.
  - `FLUSH -> call do_FLUSH()`.
  - `ALLOC_ARRAYS/CLEAR_ARRAY/... -> do_ALLOC_ARRAYS(...)` и т.д. (или развернуть в IR).

- Условные переходы:
  - `BR_COND rcond label_true`: генерируется `br i1` (читаем регистр, усечём до `i1`).
  - `BR label`: безусловный `br`.

## Пример структуры цикла
```
entry:
    XOR x1 x1 x1       ; x1 = 0
main_loop:
    XOR x2 x2 x2       ; x2 = 0
loop_y:
    XOR x5 x5 x5       ; x5 = 0
    MUL x3 x2 x1
loop_x:
    MUL x6 x3 x5
    SUBi x6 x6 16777216
    PUT_PIXEL x5 x2 x6
    INC_NEi x4 x5 512
    BR_COND x4 loop_x
    INC_NEi x4 x2 256
    BR_COND x4 loop_y
    FLUSH
    INC_NEi x4 x1 1000
    BR_COND x4 main_loop
exit:
    EXIT
```

## Применение к Game of Life
- Для рисования используется `DRAW_CELL_4x4 + SELECT_COLOR + FLUSH`.
- Для вычисления поколения — макро `COUNT_NEIGHBORS` + `GAME_OF_LIFE_RULE`, запись через `SET_CELL`, копирование `COPY_ARRAY`.
- Циклы строятся через `INC_NEi` + `BR_COND` по образцу выше.
