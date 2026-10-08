/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
.text
.globl cs_x64_install
cs_x64_install:
    cli
    cld
    movq %rcx,cs_x64_active_state(%rip)
    lgdt 40(%rcx)
    pushq $8
    leaq cs_x64_reload(%rip),%rax
    pushq %rax
    lretq
.globl cs_x64_reload
cs_x64_reload:
    movw $16,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%ss
    xorw %ax,%ax
    lldt %ax
    movw $24,%ax
    ltr %ax
    lidt 50(%rcx)
    movl $1,240(%rcx)
    ret

.p2align 5
.globl cs_x64_vector_base
cs_x64_vector_base:
.set cs_vector,0
.rept 256
    .if cs_vector!=8 && cs_vector!=10 && cs_vector!=11 && cs_vector!=12 && cs_vector!=13 && cs_vector!=14 && cs_vector!=17 && cs_vector!=21
        pushq $0
    .endif
    /* Force imm32 even for small vectors: no sign-extension ambiguity at 128. */
    .byte 0x68
    .long cs_vector
    .byte 0xE9
    .long cs_x64_fault-.-4
    .balign 32,0x90
    .set cs_vector,cs_vector+1
.endr
.globl cs_x64_vector_end
cs_x64_vector_end:

.globl cs_x64_fault
cs_x64_fault:
    cli
    cld
    movq %cr2,%r8
    movq cs_x64_active_state(%rip),%rcx
    xorl %eax,%eax
    movl $1,%edx
    lock cmpxchgl %edx,176(%rcx)
    jne cs_native_halt
    movl (%rsp),%eax
    movl %eax,180(%rcx)
    movq 8(%rsp),%rax
    movq %rax,184(%rcx)
    movq 16(%rsp),%rax
    movq %rax,192(%rcx)
    movq 24(%rsp),%rax
    movq %rax,200(%rcx)
    movq 32(%rsp),%rax
    movq %rax,208(%rcx)
    movq 40(%rsp),%rax
    movq %rax,216(%rcx)
    movq 48(%rsp),%rax
    movq %rax,224(%rcx)
    movq %r8,232(%rcx)
    movl $2,176(%rcx)
    jmp cs_native_halt

.data
.p2align 3
.globl cs_x64_active_state
cs_x64_active_state:
    .quad 0
