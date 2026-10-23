/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Windows x64: RCX = const uint64_t values[8]. Never returns. */
    .text
    .globl cs_pc_input_probe_halt
    .globl cs_pc_input_probe_spin
    .def cs_pc_input_probe_halt; .scl 2; .type 32; .endef
    .p2align 4
cs_pc_input_probe_halt:
    movq %rcx, %rax
    movq 0(%rax), %r8
    movq 8(%rax), %r9
    movq 16(%rax), %r10
    movq 24(%rax), %r11
    movq 32(%rax), %r12
    movq 40(%rax), %r13
    movq 48(%rax), %r14
    movq 56(%rax), %r15
cs_pc_input_probe_spin:
    pause
    jmp cs_pc_input_probe_spin
