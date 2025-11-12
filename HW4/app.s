	.build_version macos, 15, 0	sdk_version 15, 5
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_initEmptyField                 ; -- Begin function initEmptyField
	.p2align	2
_initEmptyField:                        ; @initEmptyField
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	x0, [sp, #8]
	str	w1, [sp, #4]
	str	wzr, [sp]
	b	LBB0_1
LBB0_1:                                 ; =>This Inner Loop Header: Depth=1
	ldr	w8, [sp]
	ldr	w9, [sp, #4]
	ldr	w10, [sp, #4]
	mul	w9, w9, w10
	subs	w8, w8, w9
	b.ge	LBB0_4
	b	LBB0_2
LBB0_2:                                 ;   in Loop: Header=BB0_1 Depth=1
	ldr	x9, [sp, #8]
	ldrsw	x10, [sp]
	mov	w8, #0                          ; =0x0
	str	w8, [x9, x10, lsl #2]
	b	LBB0_3
LBB0_3:                                 ;   in Loop: Header=BB0_1 Depth=1
	ldr	w8, [sp]
	add	w8, w8, #1
	str	w8, [sp]
	b	LBB0_1
LBB0_4:
	add	sp, sp, #16
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_countNeighbours                ; -- Begin function countNeighbours
	.p2align	2
_countNeighbours:                       ; @countNeighbours
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	w0, [x29, #-4]
	stur	w1, [x29, #-8]
	stur	x2, [x29, #-16]
	stur	w3, [x29, #-20]
	str	wzr, [sp, #24]
	mov	w8, #-1                         ; =0xffffffff
	str	w8, [sp, #20]
	b	LBB1_1
LBB1_1:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB1_3 Depth 2
	ldr	w8, [sp, #20]
	subs	w8, w8, #2
	b.ge	LBB1_13
	b	LBB1_2
LBB1_2:                                 ;   in Loop: Header=BB1_1 Depth=1
	ldur	w8, [x29, #-4]
	ldr	w9, [sp, #20]
	add	w8, w8, w9
	str	w8, [sp, #16]
	mov	w8, #-1                         ; =0xffffffff
	str	w8, [sp, #12]
	b	LBB1_3
LBB1_3:                                 ;   Parent Loop BB1_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	w8, [sp, #12]
	subs	w8, w8, #2
	b.ge	LBB1_11
	b	LBB1_4
LBB1_4:                                 ;   in Loop: Header=BB1_3 Depth=2
	ldur	w8, [x29, #-8]
	ldr	w9, [sp, #12]
	add	w8, w8, w9
	str	w8, [sp, #8]
	ldur	w8, [x29, #-4]
	ldr	w9, [sp, #16]
	subs	w8, w8, w9
	b.ne	LBB1_7
	b	LBB1_5
LBB1_5:                                 ;   in Loop: Header=BB1_3 Depth=2
	ldur	w8, [x29, #-8]
	ldr	w9, [sp, #8]
	subs	w8, w8, w9
	b.ne	LBB1_7
	b	LBB1_6
LBB1_6:                                 ;   in Loop: Header=BB1_3 Depth=2
	b	LBB1_10
LBB1_7:                                 ;   in Loop: Header=BB1_3 Depth=2
	ldur	x0, [x29, #-16]
	ldr	w1, [sp, #16]
	ldr	w2, [sp, #8]
	ldur	w3, [x29, #-20]
	bl	_getCellValue
	subs	w8, w0, #1
	b.ne	LBB1_9
	b	LBB1_8
LBB1_8:                                 ;   in Loop: Header=BB1_3 Depth=2
	ldr	w8, [sp, #24]
	add	w8, w8, #1
	str	w8, [sp, #24]
	b	LBB1_9
LBB1_9:                                 ;   in Loop: Header=BB1_3 Depth=2
	b	LBB1_10
LBB1_10:                                ;   in Loop: Header=BB1_3 Depth=2
	ldr	w8, [sp, #12]
	add	w8, w8, #1
	str	w8, [sp, #12]
	b	LBB1_3
LBB1_11:                                ;   in Loop: Header=BB1_1 Depth=1
	b	LBB1_12
LBB1_12:                                ;   in Loop: Header=BB1_1 Depth=1
	ldr	w8, [sp, #20]
	add	w8, w8, #1
	str	w8, [sp, #20]
	b	LBB1_1
LBB1_13:
	ldr	w0, [sp, #24]
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	add	sp, sp, #64
	ret
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function getCellValue
_getCellValue:                          ; @getCellValue
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	str	x0, [sp, #24]
	str	w1, [sp, #20]
	str	w2, [sp, #16]
	str	w3, [sp, #12]
	ldr	w8, [sp, #20]
	ldr	w9, [sp, #12]
	add	w8, w8, w9
	ldr	w10, [sp, #12]
	sdiv	w9, w8, w10
	mul	w9, w9, w10
	subs	w8, w8, w9
	str	w8, [sp, #20]
	ldr	w8, [sp, #16]
	ldr	w9, [sp, #12]
	add	w8, w8, w9
	ldr	w10, [sp, #12]
	sdiv	w9, w8, w10
	mul	w9, w9, w10
	subs	w8, w8, w9
	str	w8, [sp, #16]
	ldr	x8, [sp, #24]
	ldr	w9, [sp, #20]
	ldr	w10, [sp, #12]
	mul	w9, w9, w10
	ldr	w10, [sp, #16]
	add	w9, w9, w10
	ldr	w0, [x8, w9, sxtw #2]
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_stepLife                       ; -- Begin function stepLife
	.p2align	2
_stepLife:                              ; @stepLife
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	w0, [x29, #-4]
	stur	x1, [x29, #-16]
	str	x2, [sp, #24]
	str	wzr, [sp, #20]
	b	LBB3_1
LBB3_1:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB3_3 Depth 2
	ldr	w8, [sp, #20]
	ldur	w9, [x29, #-4]
	subs	w8, w8, w9
	b.ge	LBB3_13
	b	LBB3_2
LBB3_2:                                 ;   in Loop: Header=BB3_1 Depth=1
	str	wzr, [sp, #16]
	b	LBB3_3
LBB3_3:                                 ;   Parent Loop BB3_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	w8, [sp, #16]
	ldur	w9, [x29, #-4]
	subs	w8, w8, w9
	b.ge	LBB3_11
	b	LBB3_4
LBB3_4:                                 ;   in Loop: Header=BB3_3 Depth=2
	ldr	w0, [sp, #16]
	ldr	w1, [sp, #20]
	ldur	x2, [x29, #-16]
	ldur	w3, [x29, #-4]
	bl	_countNeighbours
	str	w0, [sp, #12]
	ldr	w8, [sp, #16]
	ldur	w9, [x29, #-4]
	mul	w8, w8, w9
	ldr	w9, [sp, #20]
	add	w8, w8, w9
	str	w8, [sp, #8]
	ldur	x8, [x29, #-16]
	ldrsw	x9, [sp, #8]
	ldr	w8, [x8, x9, lsl #2]
	subs	w8, w8, #1
	b.ne	LBB3_8
	b	LBB3_5
LBB3_5:                                 ;   in Loop: Header=BB3_3 Depth=2
	ldr	w9, [sp, #12]
	mov	w8, #1                          ; =0x1
	subs	w9, w9, #2
	str	w8, [sp, #4]                    ; 4-byte Folded Spill
	b.eq	LBB3_7
	b	LBB3_6
LBB3_6:                                 ;   in Loop: Header=BB3_3 Depth=2
	ldr	w8, [sp, #12]
	subs	w8, w8, #3
	cset	w8, eq
	str	w8, [sp, #4]                    ; 4-byte Folded Spill
	b	LBB3_7
LBB3_7:                                 ;   in Loop: Header=BB3_3 Depth=2
	ldr	w9, [sp, #4]                    ; 4-byte Folded Reload
	mov	w8, #0                          ; =0x0
	and	w9, w9, #0x1
	ands	w9, w9, #0x1
	csinc	w8, w8, wzr, eq
	ldr	x9, [sp, #24]
	ldrsw	x10, [sp, #8]
	str	w8, [x9, x10, lsl #2]
	b	LBB3_9
LBB3_8:                                 ;   in Loop: Header=BB3_3 Depth=2
	ldr	w9, [sp, #12]
	mov	w8, #0                          ; =0x0
	subs	w9, w9, #3
	csinc	w8, w8, wzr, ne
	ldr	x9, [sp, #24]
	ldrsw	x10, [sp, #8]
	str	w8, [x9, x10, lsl #2]
	b	LBB3_9
LBB3_9:                                 ;   in Loop: Header=BB3_3 Depth=2
	b	LBB3_10
LBB3_10:                                ;   in Loop: Header=BB3_3 Depth=2
	ldr	w8, [sp, #16]
	add	w8, w8, #1
	str	w8, [sp, #16]
	b	LBB3_3
LBB3_11:                                ;   in Loop: Header=BB3_1 Depth=1
	b	LBB3_12
LBB3_12:                                ;   in Loop: Header=BB3_1 Depth=1
	ldr	w8, [sp, #20]
	add	w8, w8, #1
	str	w8, [sp, #20]
	b	LBB3_1
LBB3_13:
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	add	sp, sp, #64
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_randomizeField                 ; -- Begin function randomizeField
	.p2align	2
_randomizeField:                        ; @randomizeField
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	x0, [x29, #-8]
	stur	w1, [x29, #-12]
	str	wzr, [sp, #16]
	b	LBB4_1
LBB4_1:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB4_3 Depth 2
	ldr	w8, [sp, #16]
	ldur	w9, [x29, #-12]
	subs	w8, w8, w9
	b.ge	LBB4_8
	b	LBB4_2
LBB4_2:                                 ;   in Loop: Header=BB4_1 Depth=1
	str	wzr, [sp, #12]
	b	LBB4_3
LBB4_3:                                 ;   Parent Loop BB4_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	w8, [sp, #12]
	ldur	w9, [x29, #-12]
	subs	w8, w8, w9
	b.ge	LBB4_6
	b	LBB4_4
LBB4_4:                                 ;   in Loop: Header=BB4_3 Depth=2
	bl	_simRand
	mov	w9, #5                          ; =0x5
	sdiv	w8, w0, w9
	mul	w8, w8, w9
	subs	w9, w0, w8
	mov	w8, #0                          ; =0x0
	subs	w9, w9, #0
	csinc	w8, w8, wzr, ne
	str	w8, [sp, #8]
	ldr	w8, [sp, #8]
	ldur	x9, [x29, #-8]
	ldr	w10, [sp, #12]
	ldur	w11, [x29, #-12]
	mul	w10, w10, w11
	ldr	w11, [sp, #16]
	add	w10, w10, w11
	str	w8, [x9, w10, sxtw #2]
	b	LBB4_5
LBB4_5:                                 ;   in Loop: Header=BB4_3 Depth=2
	ldr	w8, [sp, #12]
	add	w8, w8, #1
	str	w8, [sp, #12]
	b	LBB4_3
LBB4_6:                                 ;   in Loop: Header=BB4_1 Depth=1
	b	LBB4_7
LBB4_7:                                 ;   in Loop: Header=BB4_1 Depth=1
	ldr	w8, [sp, #16]
	add	w8, w8, #1
	str	w8, [sp, #16]
	b	LBB4_1
LBB4_8:
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_app                            ; -- Begin function app
	.p2align	2
_app:                                   ; @app
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	mov	w1, #128                        ; =0x80
	stur	w1, [x29, #-16]                 ; 4-byte Folded Spill
	stur	w1, [x29, #-4]
	adrp	x0, _app.field@PAGE
	add	x0, x0, _app.field@PAGEOFF
	str	x0, [sp, #24]                   ; 8-byte Folded Spill
	bl	_initEmptyField
	ldr	x0, [sp, #24]                   ; 8-byte Folded Reload
	ldur	w1, [x29, #-16]                 ; 4-byte Folded Reload
	bl	_randomizeField
	stur	wzr, [x29, #-8]
	b	LBB5_1
LBB5_1:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB5_3 Depth 2
	ldur	w8, [x29, #-8]
	subs	w8, w8, #2000
	b.ge	LBB5_8
	b	LBB5_2
LBB5_2:                                 ;   in Loop: Header=BB5_1 Depth=1
	mov	w0, #128                        ; =0x80
	str	w0, [sp, #12]                   ; 4-byte Folded Spill
	adrp	x1, _app.field@PAGE
	add	x1, x1, _app.field@PAGEOFF
	str	x1, [sp, #16]                   ; 8-byte Folded Spill
	bl	_renderField
	ldr	w0, [sp, #12]                   ; 4-byte Folded Reload
	ldr	x1, [sp, #16]                   ; 8-byte Folded Reload
	adrp	x2, _app.nextField@PAGE
	add	x2, x2, _app.nextField@PAGEOFF
	bl	_stepLife
	stur	wzr, [x29, #-12]
	b	LBB5_3
LBB5_3:                                 ;   Parent Loop BB5_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldur	w8, [x29, #-12]
	subs	w8, w8, #4, lsl #12             ; =16384
	b.ge	LBB5_6
	b	LBB5_4
LBB5_4:                                 ;   in Loop: Header=BB5_3 Depth=2
	ldursw	x9, [x29, #-12]
	adrp	x8, _app.nextField@PAGE
	add	x8, x8, _app.nextField@PAGEOFF
	ldr	w8, [x8, x9, lsl #2]
	ldursw	x10, [x29, #-12]
	adrp	x9, _app.field@PAGE
	add	x9, x9, _app.field@PAGEOFF
	str	w8, [x9, x10, lsl #2]
	b	LBB5_5
LBB5_5:                                 ;   in Loop: Header=BB5_3 Depth=2
	ldur	w8, [x29, #-12]
	add	w8, w8, #1
	stur	w8, [x29, #-12]
	b	LBB5_3
LBB5_6:                                 ;   in Loop: Header=BB5_1 Depth=1
	b	LBB5_7
LBB5_7:                                 ;   in Loop: Header=BB5_1 Depth=1
	ldur	w8, [x29, #-8]
	add	w8, w8, #1
	stur	w8, [x29, #-8]
	b	LBB5_1
LBB5_8:
	mov	w0, #128                        ; =0x80
	adrp	x1, _app.field@PAGE
	add	x1, x1, _app.field@PAGEOFF
	bl	_renderField
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	add	sp, sp, #64
	ret
	.cfi_endproc
                                        ; -- End function
.zerofill __DATA,__bss,_app.field,65536,2 ; @app.field
.zerofill __DATA,__bss,_app.nextField,65536,2 ; @app.nextField
.subsections_via_symbols
