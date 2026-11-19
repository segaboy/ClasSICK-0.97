/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
    .text
    .global cs_mac_enter_boot_image
cs_mac_enter_boot_image:
    /* Fixed return boundary inside reserved probe RAM. A real disk may never
       return. No native call frame or accidental C register value is exposed. */
    move.w #0x60fe,0x7ffe
    move.l #0x7800,%sp
    move.l #0x7ffe,-(%sp)
    moveq #0,%d0
    moveq #0,%d1
    moveq #0,%d2
    moveq #0,%d3
    moveq #0,%d4
    moveq #0,%d5
    moveq #0,%d6
    moveq #0,%d7
    movea.l #0x8000,%a0
    suba.l %a1,%a1
    suba.l %a2,%a2
    suba.l %a3,%a3
    suba.l %a4,%a4
    suba.l %a5,%a5
    suba.l %a6,%a6
    move.w #0x2700,%sr
    jmp 0x8002
    .section .note.GNU-stack,"",@progbits
