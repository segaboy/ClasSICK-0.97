/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
    .text
#ifdef CS_DIAG_UNHANDLED
    .global __wrap_cs_mac_trap_target
__wrap_cs_mac_trap_target:
    move.l 4(%sp),-(%sp)
    jsr __real_cs_mac_trap_target
    addq.l #4,%sp
    tst.l %d0
    bne.s .Lreturn
    /* Only the lookup made by the native Line-A dispatcher is an observed
       service request. A direct guest call to this helper keeps its zero. */
    move.l (%sp),%d0
    cmpi.l #cs_mac_startup_line_a,%d0
    bcs.s .Ldirect_refusal
    cmpi.l #cs_mac_startup_service_return,%d0
    bcs.s cs_diag_gateway
.Ldirect_refusal:
    moveq #0,%d0
.Lreturn:
    rts
    .global cs_diag_gateway
cs_diag_gateway:
    move.w 6(%sp),%d0         /* Low word of the dispatcher's uint32_t argument. */
#else
    .global __wrap_cs_mac_load_startup_disk
__wrap_cs_mac_load_startup_disk:
    jsr __real_cs_mac_load_startup_disk
    tst.l %d0
    beq.s .Lreturn
    move.l #cs_diag_gateway,0x28
.Lreturn:
    rts

    .global cs_diag_gateway
cs_diag_gateway:
    /* The 68000 stacks SR at SP and the faulting instruction PC at SP+2.
       Read that one word only, as the normal vector-10 calling interface. */
    movea.l 2(%sp),%a0
    move.w (%a0),%d0
#endif
    move.w %d0,%d1
    andi.w #0xf000,%d1
    cmpi.w #0xa000,%d1
    bne.w cs_diag_unmatched
    btst #11,%d0
    beq.s .Los
    btst #9,%d0
    bne.w cs_diag_unmatched
    andi.w #0x09ff,%d0
    bra.s .Llookup
.Los:
    andi.w #0x00ff,%d0
.Llookup:
    .macro select_service key, name
    cmpi.w #\key,%d0
    beq.w cs_diag_\name
    .endm
#define CS_SERVICE(key, name) select_service key, name
#include "service-labels.def"
#undef CS_SERVICE
    bra.w cs_diag_unmatched

    /* Each terminal has a distinct, immutable PC. No guest RAM diagnostic,
       register dump, service implementation or service return is involved. */
    .macro terminal name
    .global cs_diag_\name
cs_diag_\name:
    bra.s cs_diag_\name
    .endm
#define CS_SERVICE(key, name) terminal name
#include "service-labels.def"
#undef CS_SERVICE
    terminal unmatched
    .section .note.GNU-stack,"",@progbits
