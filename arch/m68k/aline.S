/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../apps/aline-probe/program.h"
    .text
    .global cs_mac_line_a_handler
cs_mac_line_a_handler:
    suba.w #72,%sp
    movem.l %d0-%d7/%a0-%a6,(%sp)
    lea 78(%sp),%a0
    move.l %a0,60(%sp)
    move.l 74(%sp),64(%sp)
    move.w 72(%sp),68(%sp)
    move.l %sp,-(%sp)
    jsr cs_mac_line_a
    addq.l #4,%sp
    move.w 68(%sp),72(%sp)
    move.l 64(%sp),74(%sp)
    movem.l (%sp),%d0-%d7/%a0-%a6
    adda.w #72,%sp
    rte
    .global cs_mac_aline_program
    .global cs_mac_aline_program_end
cs_mac_aline_program:
    .byte CS_ALINE_PROGRAM_BYTES
cs_mac_aline_program_end:
    .global cs_mac_aline_probe
cs_mac_aline_probe:
    movem.l %d2-%d7/%a2-%a6,-(%sp)
    move.w %sr,-(%sp)
    moveq #0x11,%d1
    moveq #0x12,%d2
    moveq #0x13,%d3
    moveq #0x14,%d4
    moveq #0x15,%d5
    moveq #0x16,%d6
    moveq #0x17,%d7
    movea.l #0x21,%a1
    movea.l #0x22,%a2
    movea.l #0x23,%a3
    movea.l #0x24,%a4
    movea.l #0x25,%a5
    movea.l #0x26,%a6
    move.w #0x271F,%sr
    jsr cs_mac_aline_program
    move.w %sr,-(%sp)
    cmpi.l #32,%d0
    bne 1f
    cmpa.l #0x400100,%a0
    bne 1f
    cmpi.l #0x11,%d1
    bne 1f
    cmpi.l #0x12,%d2
    bne 1f
    cmpi.l #0x13,%d3
    bne 1f
    cmpi.l #0x14,%d4
    bne 1f
    cmpi.l #0x15,%d5
    bne 1f
    cmpi.l #0x16,%d6
    bne 1f
    cmpi.l #0x17,%d7
    bne 1f
    cmpa.l #0x21,%a1
    bne 1f
    cmpa.l #0x22,%a2
    bne 1f
    cmpa.l #0x23,%a3
    bne 1f
    cmpa.l #0x24,%a4
    bne 1f
    cmpa.l #0x25,%a5
    bne 1f
    cmpa.l #0x26,%a6
    bne 1f
    cmpi.w #0x2710,(%sp)
    bne 1f
    moveq #1,%d0
    bra 2f
1:  moveq #0,%d0
2:  addq.l #2,%sp
    move.w (%sp)+,%sr
    movem.l (%sp)+,%d2-%d7/%a2-%a6
    rts
    .section .note.GNU-stack,"",@progbits
