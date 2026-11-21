/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
    .global cs_mac_current_zone,cs_mac_application_limit,cs_mac_memory_error
    .set cs_mac_current_zone,0x0118
    .set cs_mac_application_limit,0x0130
    .set cs_mac_memory_error,0x0220
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
    beq.w cs_mac_startup_stop

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

    /* With D0-D2/A0-A1 saved, SR and the faulting PC are at 20/22(SP).
       Select the continuation before calling guest code: the routine may
       update its own entry or the caller's trap word before returning. */
    .global cs_mac_startup_line_a
cs_mac_startup_line_a:
    movem.l %d0-%d2/%a0-%a1,-(%sp)
    move.w 20(%sp),%d1
    andi.w #0xa000,%d1
    cmpi.w #0x2000,%d1
    bne.w cs_mac_startup_stop
    move.l 22(%sp),%d1
    btst #0,%d1
    bne.w cs_mac_startup_stop
    cmpi.l #0x410000,%d1
    bcc.w cs_mac_startup_stop
    movea.l %d1,%a0
    moveq #0,%d1
    move.w (%a0),%d1
    addq.l #2,22(%sp)
    btst #8,%d1
    bne.s .Lreturns_a0
    pea .Lrestore_a0
    bra.s .Llookup
.Lreturns_a0:
    pea .Lrestore_regs
.Llookup:
    move.l %d1,-(%sp)
    jsr cs_mac_trap_target
    addq.l #4,%sp
    tst.l %d0
    beq.w cs_mac_startup_stop
    move.l %d0,-(%sp)
    movem.l 8(%sp),%d0-%d2/%a0-%a1
    rts                         /* enter handler; its RTS pops our continuation */
.Lrestore_a0:
    movea.l 12(%sp),%a0
.Lrestore_regs:
    tst.w %d0
    move.w %sr,%d1
    andi.w #0x000f,%d1
    andi.w #0xfff0,20(%sp)
    or.w %d1,20(%sp)
    move.l 4(%sp),%d1
    move.l 8(%sp),%d2
    movea.l 16(%sp),%a1
    lea 20(%sp),%sp
    .global cs_mac_startup_service_return
cs_mac_startup_service_return:
    rte

    /* Actual ROM addresses returned by GetTrapAddress. The only C result
       allowed out of the direct Get wrapper is A0; direct calls retain CCR. */
    .global cs_mac_get_trap_address
cs_mac_get_trap_address:
    move.w %sr,-(%sp)
    movem.l %d0-%d1/%a1,-(%sp)
    move.l %d0,-(%sp)
    jsr cs_mac_trap_get_value
    addq.l #4,%sp
    tst.l %d0
    beq.s cs_mac_startup_stop
    movea.l %d0,%a0
    movem.l (%sp)+,%d0-%d1/%a1
    move.w (%sp)+,%ccr
    rts

    .global cs_mac_set_trap_address
cs_mac_set_trap_address:
    move.w %sr,-(%sp)
    movem.l %d0-%d1/%a0-%a1,-(%sp)
    move.l %a0,-(%sp)
    move.l %d0,-(%sp)
    jsr cs_mac_trap_set_value
    addq.l #8,%sp
    tst.l %d0
    beq.s cs_mac_startup_stop
    movem.l (%sp)+,%d0-%d1/%a0-%a1
    move.w (%sp)+,%ccr
    rts

    .global cs_mac_startup_stop
cs_mac_startup_stop:
    stop #0x2700
    bra.s cs_mac_startup_stop

    /* The directory publishes the plain entry for direct JSR. The selector
       for a mediated call comes from its actual instruction. */
    .global cs_mac_new_ptr
cs_mac_new_ptr:
    move.l #0xa11e,-(%sp)
    bra.s .Lnew_ptr
    .global cs_mac_new_ptr_clear
cs_mac_new_ptr_clear:
    move.l #0xa31e,-(%sp)
    bra.s .Lnew_ptr
    .global cs_mac_new_ptr_sys
cs_mac_new_ptr_sys:
    move.l #0xa51e,-(%sp)
    bra.s .Lnew_ptr
    .global cs_mac_new_ptr_sys_clear
cs_mac_new_ptr_sys_clear:
    move.l #0xa71e,-(%sp)
    .global cs_mac_new_ptr_dispatch
cs_mac_new_ptr_dispatch:
.Lnew_ptr:
    move.w %sr,-(%sp)
    movem.l %d0-%d1/%a1,-(%sp)
    move.l 14(%sp),-(%sp)
    move.l %d0,-(%sp)
    jsr cs_mac_allocate_startup_ptr
    lea 8(%sp),%sp
    cmpi.l #-1,%d0
    beq.w cs_mac_startup_stop
    movea.l %d0,%a0
    movem.l (%sp)+,%d0-%d1/%a1
    move.w (%sp)+,%ccr
    addq.l #4,%sp
    cmpa.l #0,%a0
    beq.s cs_mac_new_ptr_failure
    .global cs_mac_new_ptr_success
cs_mac_new_ptr_success:
    clr.w %d0
    rts
    .global cs_mac_new_ptr_failure
cs_mac_new_ptr_failure:
    move.w cs_mac_memory_error,%d0
    rts
    .section .note.GNU-stack,"",@progbits
