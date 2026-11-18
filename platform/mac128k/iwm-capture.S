/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original bounded native byte-stream capture. Parsing/checksums stay in C.
 * Contract: address[7], wire[706], expected address identity[3]. Returns 0/1.
 * Exclusive IWM access and masked interrupts are caller preconditions. */
    .text
    .global cs_mac_iwm_capture
cs_mac_iwm_capture:
    movem.l %d2-%d7/%a2-%a6,-(%sp)
    movea.l 48(%sp),%a2
    movea.l 52(%sp),%a3
    movea.l 56(%sp),%a4
    movea.l #0xc01800,%a0
    move.l #2000000,%d7

    .macro next_byte
1:  subq.l #1,%d7
    beq .Lfailure
    move.b (%a0),%d0
    bpl 1b
    .endm

.Laddress_start:
    next_byte
    cmpi.b #0xd5,%d0
    bne .Laddress_start
.Laddress_middle:
    next_byte
    cmpi.b #0xaa,%d0
    beq .Laddress_end
    cmpi.b #0xd5,%d0
    beq .Laddress_middle
    bra .Laddress_start
.Laddress_end:
    next_byte
    cmpi.b #0x96,%d0
    beq .Laddress_capture
    cmpi.b #0xd5,%d0
    beq .Laddress_middle
    bra .Laddress_start
.Laddress_capture:
    movea.l %a2,%a1
    moveq #6,%d1
.Laddress_loop:
    next_byte
    move.b %d0,(%a1)+
    dbra %d1,.Laddress_loop
    move.b (%a2),%d0
    cmp.b (%a4),%d0
    bne .Laddress_start
    move.b 1(%a2),%d0
    cmp.b 1(%a4),%d0
    bne .Laddress_start
    move.b 2(%a2),%d0
    cmp.b 2(%a4),%d0
    bne .Laddress_start

.Ldata_start:
    next_byte
    cmpi.b #0xd5,%d0
    bne .Ldata_start
.Ldata_middle:
    next_byte
    cmpi.b #0xaa,%d0
    beq .Ldata_end
    cmpi.b #0xd5,%d0
    beq .Ldata_middle
    bra .Ldata_start
.Ldata_end:
    next_byte
    cmpi.b #0xad,%d0
    beq .Ldata_capture
    cmpi.b #0xd5,%d0
    beq .Ldata_middle
    bra .Ldata_start
.Ldata_capture:
    movea.l %a3,%a1
    move.w #705,%d1
.Ldata_loop:
    next_byte
    move.b %d0,(%a1)+
    dbra %d1,.Ldata_loop
    moveq #1,%d0
    bra .Lreturn
.Lfailure:
    moveq #0,%d0
.Lreturn:
    movem.l (%sp)+,%d2-%d7/%a2-%a6
    rts
    .section .note.GNU-stack,"",@progbits
