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
#ifndef CS_X64_FAULT_PROBE
#define CS_X64_FAULT_PROBE 0
#endif
#if CS_X64_FAULT_PROBE == 6
.globl cs_x64_probe_instruction
cs_x64_probe_instruction:
    ud2
    jmp cs_x64_terminal_halt
#elif CS_X64_FAULT_PROBE == 13
    movw $0x28,%ax
.globl cs_x64_probe_instruction
cs_x64_probe_instruction:
    movw %ax,%ds
    jmp cs_x64_terminal_halt
#elif CS_X64_FAULT_PROBE != 0
#error Unsupported fault probe
#endif
.globl cs_x64_terminal_halt
cs_x64_terminal_halt:
    cli
    hlt
    jmp cs_x64_terminal_halt
.globl cs_x64_services_stop
cs_x64_services_stop:
    cli
    cld
    movl (%rcx),%r9d
    movl 4(%rcx),%r10d
    movl 8(%rcx),%r11d
    movl $0x43535331,%r14d
    .byte 0xE9
    .long cs_native_halt-.-4
.section .rdata,"dr"
.globl cs_entry_anchor
.p2align 3
cs_entry_anchor:
    .quad cs_x64_enter
