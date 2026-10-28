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

#if defined(CLASSICK_MAC_IDLE_PROBE) && CLASSICK_MAC_IDLE_PROBE && !CLASSICK_MAC_TIMER_ACTIVITY_PROBE
    /* Diagnostic noreturn halt: IPL7 explicitly held, no MMIO, no return.
     * Exposes the copied MWR1 snapshot in registers; A0-A6 are raw data
     * (A6 stays &cs_mac_trace until all record loads finish). Offsets are
     * bound by C static assertions in window-entry.c. */
    .globl cs_mac_window_idle_checkpoint
    .even
cs_mac_window_idle_checkpoint:
    ori.w   #0x0700,%sr
    movea.l #cs_mac_trace,%a6
    movea.l cs_mac_diag+24,%a5
    move.l  #0x4D575231,%d0
    move.l  28(%a6),%d1
    move.l  36(%a6),%d2
    move.l  48(%a6),%d3
    move.l  100(%a6),%d4
    move.l  104(%a6),%d5
    move.l  120(%a6),%d6
    move.l  124(%a6),%d7
    movea.l 108(%a6),%a0
    movea.l 112(%a6),%a1
    movea.l 116(%a6),%a2
    movea.l 52(%a6),%a3
    movea.l 56(%a6),%a4
    .globl cs_mac_window_idle_spin
cs_mac_window_idle_spin:
0:  bra.s 0b
#endif
    .section .note.GNU-stack,"",@progbits
