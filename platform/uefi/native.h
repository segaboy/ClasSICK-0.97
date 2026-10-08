/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_UEFI_NATIVE_H
#define CLASSICK_UEFI_NATIVE_H
#include "loader.h"

/* SPEC-0009 post-handoff presentation gate and 32-byte owned trace record. */
enum { CS_NATIVE_PRESENTED=0, CS_NATIVE_ARGUMENT=1, CS_NATIVE_NOT_EXITED=2,
    CS_NATIVE_NOT_READY=3, CS_NATIVE_TARGET=4, CS_NATIVE_ARENA=5,
    CS_NATIVE_DRAW=6 };
enum { CS_NATIVE_TRACE_MAGIC=0x31525043, CS_NATIVE_TRACE_VERSION=1,
    CS_NATIVE_TRACE_BYTES=32 };
/* Framebuffer/arena/trace are the identity views of the handoff's owned or
   validated addresses. Gates are checked before any framebuffer address use. */
uint32_t cs_native_present(const cs_uefi_handoff *handoff,uint32_t descriptors_ready,
    volatile unsigned char *framebuffer,void *arena,unsigned char *trace);
#endif
