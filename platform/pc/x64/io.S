/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* SPEC-0010: one 32-bit I/O port read; Microsoft x64 ABI port in ECX. */
.text
.globl cs_x64_inl
cs_x64_inl:
    movl %ecx,%edx
    inl %dx,%eax
    ret
