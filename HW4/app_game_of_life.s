; Game of Life на макро-ISA (комментарии обрабатываются препроцессором)
; Регистры: x0 = cur, x1 = nxt, x2 = step, x4 = y, x5 = x, остальные — временные.

entry:
    ; Выделить и очистить буферы
    ALLOC_ARRAYS x0 x1
    CLEAR_ARRAY x0 16384           ; GRID_SIZE=128 => 128*128 = 16384

; Рандомная инициализация cur
    XOR x4 x4 x4                   ; y = 0
rand_y_loop:
    XOR x5 x5 x5                   ; x = 0
rand_x_loop:
    RANDOMIZE_CELL x0 x5 x4 5      ; шанс 1/5
    INC_NEi x6 x5 128
    BR_COND x6 rand_x_loop
    INC_NEi x6 x4 128
    BR_COND x6 rand_y_loop

; Основной цикл по поколениям
    XOR x2 x2 x2                   ; step = 0
main_loop:
; Отрисовка кадра
    XOR x4 x4 x4
draw_y_loop:
    XOR x5 x5 x5
draw_x_loop:
    GET_CELL x6 x0 x5 x4
    SELECT_COLOR x7 x6 0xFFFFFFFF 0xFF000000
    DRAW_CELL_4x4 x5 x4 x7
    INC_NEi x8 x5 128
    BR_COND x8 draw_x_loop
    INC_NEi x8 x4 128
    BR_COND x8 draw_y_loop
    FLUSH

; Вычисление следующего поколения
    XOR x4 x4 x4
step_y_loop:
    XOR x5 x5 x5
step_x_loop:
    COUNT_NEIGHBORS x6 x0 x5 x4
    GET_CELL x7 x0 x5 x4
    GAME_OF_LIFE_RULE x7 x7 x6     ; x7 = new value
    SET_CELL x1 x5 x4 x7
    INC_NEi x8 x5 128
    BR_COND x8 step_x_loop
    INC_NEi x8 x4 128
    BR_COND x8 step_y_loop

; nxt -> cur
    COPY_ARRAY x0 x1 16384

; Следующее поколение
    INC_NEi x9 x2 1000
    BR_COND x9 main_loop

exit:
    EXIT
