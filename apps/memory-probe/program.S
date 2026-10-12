/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Independently authored assembler oracle for the C byte fixture. */
    .text
    .global cs_memory_fixture
cs_memory_fixture:
    movea.l #0x1800,%a0
    move.l #0x11223344,(%a0)
    move.l (%a0)+,%d1
    move.w #0xA55A,-(%a0)
    moveq #0,%d2
    bne.s .Laftercall
    bsr.s .Lsub
.Laftercall:
    move.w (%a0)+,%d2
    move.l -(%a0),%d3
    rts
.Lsub:
    move.b #0x7F,(%a0)
    rts
    .section .note.GNU-stack,"",@progbits
