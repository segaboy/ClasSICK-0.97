/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original disk callers for both targets. A4 advances through a result block
   read independently by BootHush. Only a completed caller writes $AC. */
    .macro expect branch
    \branch 1f
    illegal
1:
    .endm

    .macro finish
    move.b #0xac,(%a4)
1:  bra.s 1b
    .endm

    .section .allocations,"ax"
    movea.l #0x9100,%a4
    move.w #0x5a3c,0x220           /* Poison MemErr before the first allocation. */
    moveq #0x1e,%d0
    .word 0xa146                   /* Query the installed native NewPtr. */
    move.l %a0,(%a4)+
    moveq #5,%d0
    ori.b #0x1f,%ccr
    .word 0xa11e
    expect beq.s                   /* noErr clears N/V/C and sets Z. */
    expect bpl.s
    expect bvc.s
    expect bcc.s
    move.l %a0,(%a4)+
    movea.l %a0,%a2
    move.l #0x11223344,(%a0)
    move.b #0x55,4(%a0)
    move.l #0x77777777,0x10086     /* Poison the following clear request. */
    moveq #3,%d0
    .word 0xa31e
    move.l %a0,(%a4)+
    move.w (%a0),(%a4)+
    move.b 2(%a0),(%a4)+
    move.b 4(%a2),(%a4)+
    move.l (%a2),(%a4)+

    moveq #8,%d0
    .word 0xa51e
    move.l %a0,(%a4)+
    movea.l %a0,%a2
    move.b #0xc7,7(%a0)
    move.l #0x66666666,0xa088
    move.b #0x66,0xa08c
    moveq #5,%d0
    .word 0xa71e
    move.l %a0,(%a4)+
    move.l (%a0),(%a4)+
    move.b 4(%a0),(%a4)+
    move.b 7(%a2),(%a4)+
    move.w 0x220,(%a4)+
    move.l %d0,(%a4)+
    move.l %sp,(%a4)+
    finish

    .section .limits,"ax"
    movea.l #0x9100,%a4
    move.w #0x5a3c,0x220
    moveq #-1,%d0
    .word 0xa11e
    expect bmi.s
    expect bne.s
    expect bcc.s
    expect bvc.s
    move.l %d0,(%a4)+
    move.l %a0,(%a4)+
    move.w 0x220,(%a4)+
    move.l #0x12345678,%d0
    .word 0xa31e
    move.l %d0,(%a4)+              /* D0 high word survives memFullErr. */
    move.l %a0,(%a4)+
    move.w 0x220,(%a4)+
    moveq #0,%d0
    .word 0xa11e                   /* The refused requests used no bytes. */
    move.l %a0,(%a4)+
    move.l %d0,(%a4)+
    move.w 0x220,(%a4)+
    move.b #0xba,0x10081           /* Preserve the preceding allocation. */
    move.b #0x77,0x17fff           /* Last and first bytes of two 286 banks. */
    move.b #0x77,0x18000
    move.b #0xce,0x1a700           /* Immediately beyond the app heap. */
    move.l #0xa67e,%d0              /* Exact app space left after zero-size. */
    .word 0xa31e
    move.l %a0,(%a4)+
    move.b 0x10081,(%a4)+
    move.b 0x17fff,(%a4)+
    move.b 0x18000,(%a4)+
    move.b 0x1a700,(%a4)+
    move.l %a0,%d1
    addi.l #0xa67d,%d1
    movea.l %d1,%a2
    move.b #0xee,(%a2)
    move.b 0x1a6ff,(%a4)+
    clr.b (%a4)+
    moveq #1,%d0
    .word 0xa11e                   /* No app space remains. */
    move.l %d0,(%a4)+
    move.l %a0,(%a4)+
    move.w 0x220,(%a4)+
    moveq #2,%d0
    .word 0xa51e                   /* The system zone is still available. */
    move.l %a0,(%a4)+
    move.w 0x220,(%a4)+
    move.l #0xa000,0x118           /* Plain NewPtr in the current SysZone. */
    moveq #1,%d0
    .word 0xa11e
    move.l %a0,(%a4)+
    finish

    .section .patching,"ax"
    movea.l #0x9100,%a4
    moveq #0x1e,%d0
    .word 0xa146
    movea.l %a0,%a2
    lea .Lreplacement(%pc),%a0
    .word 0xa047
    moveq #3,%d0
    .word 0xa11e
    move.l %a0,(%a4)+
    move.l %d0,(%a4)+
    moveq #0x1e,%d0
    movea.l %a2,%a0
    .word 0xa047
    moveq #3,%d0
    .word 0xa11e
    move.l %a0,(%a4)+
    move.l %d0,(%a4)+
    finish
.Lreplacement:
    movea.l #0x9400,%a0
    move.l #0x3456,%d0
    rts
