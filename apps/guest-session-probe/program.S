/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
.section .text
.globl cs_guest_session_program
cs_guest_session_program:
    moveq #17,%d0
    move.w #0x1234,-(%sp)
    move.w (%sp)+,%d1
    bsr.w session_sub
    rts
session_sub:
    moveq #42,%d2
    rts
