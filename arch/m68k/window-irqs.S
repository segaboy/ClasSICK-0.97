/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original GNU 68000 interrupt veneers. Each raises IPL to 7 before any
 * shared-state touch, saves D0-D7/A0-A6, passes one 32-bit source argument
 * (1 VIA, 2 SCC, 3 combined) to cs_mac_window_interrupt, restores, RTE.
 * No vector installation here. Never executed by host tests. */

    .text
    .extern cs_mac_window_interrupt

    .macro CS_WINDOW_VENEER name, source
    .globl \name
    .even
\name:
    ori.w   #0x0700,%sr
    movem.l %d0-%d7/%a0-%a6,-(%sp)
    move.l  #\source,-(%sp)
    jsr     cs_mac_window_interrupt
    addq.l  #4,%sp
    movem.l (%sp)+,%d0-%d7/%a0-%a6
    rte
    .endm

    CS_WINDOW_VENEER cs_mac_window_via_irq, 1
    CS_WINDOW_VENEER cs_mac_window_scc_irq, 2
    CS_WINDOW_VENEER cs_mac_window_combined_irq, 3

    /* Quiescent IPL7 caller only. Absolute vector RAM stores belong to this
     * native boundary, rather than C pointers close to the null address. */
    .globl cs_mac_window_install_vectors
cs_mac_window_install_vectors:
    move.l #cs_mac_window_via_irq,0x64
    move.l #cs_mac_window_scc_irq,0x68
    move.l #cs_mac_window_combined_irq,0x6C
    rts

    .globl cs_mac_line_a_handler
cs_mac_line_a_handler:
    jmp cs_mac_fault
    .section .note.GNU-stack,"",@progbits
