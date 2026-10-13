/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original OS/core fixture; synthetic typed routine, not a CODE application. */
.text
.global classick_toolbox_fixture
classick_toolbox_fixture:
    move.l #0,-(%sp)
    move.w #0x1234,-(%sp)
    move.l #0x89abcdef,-(%sp)
    move.w #0x0101,-(%sp)
    move.w #0xa542,-(%sp)
    .word 0xa8f0
    move.l (%sp)+,%d0
    rts
