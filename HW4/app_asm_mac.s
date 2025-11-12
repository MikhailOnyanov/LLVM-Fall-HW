    .text
    .globl  _app
    .p2align 2
_app:
    // prologue
    stp     x29, x30, [sp, #-16]!
    mov     x29, sp

    // call _simInit()
    bl      _simInit

    // initEmptyField(field, size);
    adrp    x0, _field@PAGE
    add     x0, x0, _field@PAGEOFF
    mov     x1, #128              // GRID_SIZE (подставлено из sim.h)
    bl      _initEmptyField

    // randomizeField(field, size);
    adrp    x0, _field@PAGE
    add     x0, x0, _field@PAGEOFF
    mov     x1, #128
    bl      _randomizeField

    // generations counter in x9
    mov     x9, #2000

gen_loop:
    // renderField(size, field);
    mov     x0, #128
    adrp    x1, _field@PAGE
    add     x1, x1, _field@PAGEOFF
    bl      _renderField

    // stepLife(size, field, nextField);
    mov     x0, #128
    adrp    x1, _field@PAGE
    add     x1, x1, _field@PAGEOFF
    adrp    x2, _nextField@PAGE
    add     x2, x2, _nextField@PAGEOFF
    bl      _stepLife

    // copy loop: for (i=0; i < SIZE_SQ; ++i) field[i] = nextField[i];
    // rcount in x3, dst ptr x4, src ptr x5, tmp w6
    mov     x3, #16384            // 128*128 (SIZE_SQ)
    adrp    x4, _field@PAGE
    add     x4, x4, _field@PAGEOFF
    adrp    x5, _nextField@PAGE
    add     x5, x5, _nextField@PAGEOFF

copy_loop:
    cbz     x3, copy_done
    ldr     w6, [x5], #4
    str     w6, [x4], #4
    subs    x3, x3, #1
    b.ne    copy_loop

copy_done:
    subs    x9, x9, #1
    b.ne    gen_loop

    // final render
    mov     x0, #128
    adrp    x1, _field@PAGE
    add     x1, x1, _field@PAGEOFF
    bl      _renderField

    // simExit
    bl      _simExit

    // epilogue
    mov     sp, x29
    ldp     x29, x30, [sp], #16
    ret

    // --- bss/data for buffers ---
    .section __DATA,__bss

    .globl  _field
_field:
    .zero   65536      // SIZE_SQ * 4 = 128*128*4 = 65536 bytes
    .globl  _nextField
_nextField:
    .zero   65536
