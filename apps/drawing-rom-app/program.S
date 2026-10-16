/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Independently authored SPEC-0036 program. */
.text
.globl cs_drawing_app_asm
cs_drawing_app_asm:
    movea.l %a5,%a0
    move.l #0x00380078,-(%a0)
    move.l #0x00080008,-(%a0)
    move.l %a0,-(%sp)
    .word 0xA8A2
    movea.l %a5,%a0
    move.l #0x00300070,-(%a0)
    move.l #0x00100010,-(%a0)
    move.l %a0,-(%sp)
    .word 0xA8A3
    moveq #97,%d0
    rts
