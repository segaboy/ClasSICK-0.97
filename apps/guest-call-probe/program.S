/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
.section .text
.globl cs_guest_call_program
cs_guest_call_program:
    moveq #17,%d0
    jsr 0x6010:l
    jmp return_point(%pc)
    .word 0x4afc
return_point:
    rts
    moveq #42,%d1
    rts
