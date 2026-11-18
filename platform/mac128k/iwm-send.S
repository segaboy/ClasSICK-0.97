/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original bounded modeled sector writer. No physical timing claim. */
    .text
    .global cs_mac_iwm_send
cs_mac_iwm_send:
    movem.l %d2-%d4/%a2-%a3,-(%sp)
    movea.l 24(%sp),%a2
    movea.l #0xc01800,%a0
    movea.l #0xc01a01,%a1
    movea.l #0xc01c00,%a3
    move.l #2000000,%d4
    move.b 0x200(%a0),%d0
    move.b 0x600(%a0),%d0

    .macro ready
1:  subq.l #1,%d4
    beq .Ltimeout
    move.b (%a0),%d0
    btst #6,%d0
    beq .Lrejected
    btst #7,%d0
    beq 1b
    .endm

    ready
    move.b #0xd5,(%a1)
    ready
    move.b #0xaa,(%a1)
    ready
    move.b #0xad,(%a1)
    move.w #705,%d2
.Ldata:
    ready
    move.b (%a2)+,(%a1)
    dbra %d2,.Ldata
    ready
    moveq #1,%d0
    bra .Lclose
.Ltimeout:
    moveq #0,%d0
    bra .Lclose
.Lrejected:
    moveq #-1,%d0
.Lclose:
    move.b (%a3),%d1
    movem.l (%sp)+,%d2-%d4/%a2-%a3
    rts
    .section .note.GNU-stack,"",@progbits
