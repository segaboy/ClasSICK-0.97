/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
    .section .vectors,"a"
    .global cs_mac_startup_vectors
cs_mac_startup_vectors:
    .long 0x4000,cs_mac_reset
    .rept 8
    .long cs_mac_startup_stop
    .endr
    .long cs_mac_startup_line_a
    .rept 53
    .long cs_mac_startup_stop
    .endr
    .section .seal,"a"
    .long 0

    .text
    .global cs_mac_reset
cs_mac_reset:
    move.w #0x2700,%sr
    /* Reset enters at the permanent ROM address. Release the low overlay
       before publishing the RAM exception vectors or using the C stack. */
    move.b #0x48,0xEFFFFE
    move.b #0x58,0xEFE7FE
    lea cs_mac_startup_vectors,%a0
    suba.l %a1,%a1
    moveq #63,%d0
.Lvectors:
    move.l (%a0)+,(%a1)+
    dbra %d0,.Lvectors
    lea __data_load,%a0
    lea __data_start,%a1
    lea __data_end,%a2
.Lcopy:
    cmpa.l %a2,%a1
    bcc.s .Lzero_start
    move.b (%a0)+,(%a1)+
    bra.s .Lcopy
.Lzero_start:
    lea __bss_start,%a1
    lea __bss_end,%a2
.Lzero:
    cmpa.l %a2,%a1
    bcc.s .Lload
    clr.b (%a1)+
    bra.s .Lzero
.Lload:
    jsr cs_mac_load_startup_disk
    tst.l %d0
    beq.s cs_mac_startup_stop

    /* Development handoff profile. The disk takes control directly. */
    move.l #0x7800,%sp
    movea.l #0x8000,%a0
    suba.l %a1,%a1
    suba.l %a2,%a2
    suba.l %a3,%a3
    suba.l %a4,%a4
    suba.l %a5,%a5
    suba.l %a6,%a6
    moveq #0,%d0
    moveq #0,%d1
    moveq #0,%d2
    moveq #0,%d3
    moveq #0,%d4
    moveq #0,%d5
    moveq #0,%d6
    moveq #0,%d7
    move.w #0x2700,%sr
    jmp 0x8002

    /* Keep the unimplemented Line-A vector identifiable in the link map. */
    .global cs_mac_startup_line_a
cs_mac_startup_line_a:
    nop
    .global cs_mac_startup_stop
cs_mac_startup_stop:
    stop #0x2700
    bra.s cs_mac_startup_stop
    .section .note.GNU-stack,"",@progbits
