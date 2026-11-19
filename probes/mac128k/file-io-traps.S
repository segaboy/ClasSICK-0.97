/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
    .text
    .global cs_file_probe_trap
cs_file_probe_trap:
    movea.l 4(%sp),%a0
    cmpi.l #0xa003,8(%sp)
    beq .Lwrite
    .word 0xa002
    rts
.Lwrite:
    .word 0xa003
    rts
    .global cs_mac_line_a_handler
cs_mac_line_a_handler:
    movem.l %d1-%d2/%a0-%a2,-(%sp)
    movea.l 22(%sp),%a1
    moveq #0,%d0
    move.w (%a1),%d0
    addq.l #2,22(%sp)
    move.l %a0,-(%sp)
    move.l %d0,-(%sp)
    jsr cs_file_probe_service
    addq.l #8,%sp
    movem.l (%sp)+,%d1-%d2/%a0-%a2
    rte
    .section .note.GNU-stack,"",@progbits
