/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
.text
.globl cs_x64_enter
cs_x64_enter:
    cli
    cld
    movq %rdx,%rsp
    andq $-16,%rsp
    xorl %ebp,%ebp
    subq $32,%rsp
    call cs_native_stop
.globl cs_native_halt
cs_native_halt:
    cli
    hlt
    jmp cs_native_halt
.section .rdata,"dr"
.globl cs_entry_anchor
.p2align 3
cs_entry_anchor:
    .quad cs_x64_enter
