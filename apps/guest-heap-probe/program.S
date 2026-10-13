/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original guest program; no manual example or Apple application input. */
    .text
    .global cs_guest_heap_fixture
cs_guest_heap_fixture:
    moveq #16,%d0
    .word 0xA322
    movea.l %a0,%a2
    .word 0xA029
    movea.l (%a2),%a1
    move.l #0x11223344,(%a1)
    move.l (%a1),%d2
    .word 0xA025
    move.l %d0,%d3
    .word 0xA02A
    moveq #24,%d0
    .word 0xA024
    .word 0xA025
    move.l %d0,%d4
    .word 0xA029
    movea.l (%a2),%a1
    move.l (%a1),%d5
    .word 0xA02A
    movea.l (%a2),%a0
    movea.l #0x3400,%a1
    moveq #4,%d0
    .word 0xA02E
    movea.l %a2,%a0
    .word 0xA023
    moveq #16,%d0
    .word 0xA71E
    movea.l %a0,%a3
    .word 0xA021
    moveq #8,%d0
    .word 0xA020
    .word 0xA021
    move.l %d0,%d6
    .word 0xA01F
    move.l #0x47545231,%d7
    rts
    .section .note.GNU-stack,"",@progbits
