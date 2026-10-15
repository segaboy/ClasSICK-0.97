/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original four-instruction application. */
    .text
    movea.l %a5,%a0
    move.l #0x5A5A5A5A,-(%a0)
    moveq #97,%d0
    rts
    .section .note.GNU-stack,"",@progbits
