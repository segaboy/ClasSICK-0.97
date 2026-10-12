/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
    .section .vectors,"a"
    .global cs_mac_vectors
cs_mac_vectors:
    .long 0x4000
    .long cs_mac_reset
    .rept 30
    .long cs_mac_fault
    .endr
    .long cs_mac_probe_handler
    .rept 31
    .long cs_mac_fault
    .endr
    .section .seal,"a"
    .long 0
    .text
    .global cs_mac_reset
cs_mac_reset:
    move.w #0x2700,%sr
    move.b #0x48,0xEFFFFE
    move.b #0x58,0xEFE7FE
    lea cs_mac_vectors,%a0
    suba.l %a1,%a1
    moveq #63,%d0
1:  move.l (%a0)+,(%a1)+
    dbra %d0,1b
    lea __data_load,%a0
    lea __data_start,%a1
    lea __data_end,%a2
2:  cmpa.l %a2,%a1
    bcc 3f
    move.b (%a0)+,(%a1)+
    bra 2b
3:  lea __bss_start,%a1
    lea __bss_end,%a2
4:  cmpa.l %a2,%a1
    bcc 5f
    clr.b (%a1)+
    bra 4b
5:  jsr cs_mac_main
    bra cs_mac_fault
    .global cs_mac_probe
cs_mac_probe:
    trap #0
    rts
cs_mac_probe_handler:
    addq.l #1,cs_mac_diag+16
    rte
    .global cs_mac_fault
cs_mac_fault:
    move.w #0x2700,%sr
    move.l #0xF001,cs_mac_diag+24
6:  bra 6b
    .section .note.GNU-stack,"",@progbits
