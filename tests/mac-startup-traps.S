/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Independently authored boot-disk caller. Assembled into the two startup
   blocks only by the host test; all result bytes belong to this fixture. */
    .text
    move.l %sp,0x9100
    moveq #0x46,%d0
    moveq #17,%d1
    moveq #18,%d2
    moveq #19,%d3
    movea.l #0x11112222,%a1
    move.w #0x271f,%sr
    .word 0xa146
    move.w %sr,0x9104
    move.l %a0,0x9106
    movea.l %a0,%a2

    moveq #0x47,%d0
    move.w #0x271f,%sr
    jsr (%a2)                   /* direct Get returns the actual Set routine */
    move.w %sr,0x910a
    move.l %a0,0x910c
    movea.l %a0,%a3

    moveq #0,%d0
    lea .Lpatch(%pc),%a0
    move.w #0x271b,%sr
    jsr (%a3)                   /* direct Set installs slot 0 */
    move.w %sr,0x9110
    movea.l #0x12345678,%a0
    .word 0xa046
    move.w %sr,0x9112           /* low D0=0; Z=1, X retained */
    move.l %a0,0x9114

    move.w #0x2710,%sr
    .word 0xa000                /* OS handler has no A0 result */
    move.w %sr,0x9118
    move.l %a0,0x911a
    move.w #0x2710,%sr
    .word 0xa100                /* same slot, returns A0 */
    move.w %sr,0x911e
    move.l %a0,0x9120
    move.l %d0,0x9124
    move.l %d1,0x9128
    move.l %d2,0x912c
    move.l %d3,0x9130
    move.l %a1,0x9134
    move.l %sp,0x9138
    move.b #0xbc,0x913c
.Lend:
    bra.s .Lend
.Lpatch:
    /* Prove nesting: reenter Get from inside the installed handler. */
    moveq #0x47,%d0
    .word 0xa146
    move.l %a0,0x9140
    moveq #33,%d1
    moveq #34,%d2
    movea.l #0x87654320,%a1
    movea.l #0xabcdef,%a0
    moveq #-1,%d0
    rts

    .section .new_ptr,"ax"
    moveq #0x1e,%d0
    .word 0xa146
    movea.l %a0,%a2               /* Save the ROM address supplied to guest code. */
    move.l #0x55a3ffff,0x10080
    moveq #17,%d1
    moveq #18,%d2
    movea.l #0x11223344,%a1
    moveq #3,%d0
    move.w #0x271f,%sr
    jsr (%a2)
    move.w %sr,0x9100
    move.l %d0,0x9102
    move.l %a0,0x9106
    move.w 0x220,0x910a
    move.l (%a0),0x910c           /* Plain allocation leaves all four bytes. */
    move.l %d1,0x9110
    move.l %a1,0x9114
    move.l %d2,0x9118
    move.l %sp,0x911c
    move.l #0x12340000,%d0
    move.w #0x270f,%sr
    jsr (%a2)
    move.w %sr,0x9120
    move.l %d0,0x9122
    move.l %a0,0x9126
    move.w 0x220,0x912a
    move.b #0xbc,0x913c
1:  bra.s 1b
