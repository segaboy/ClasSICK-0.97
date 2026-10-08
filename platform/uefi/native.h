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

/* SPEC-0010 PM timer probe and 48-byte owned trace record. */
enum { CS_TIMER_OK=0, CS_TIMER_ARGUMENT=1, CS_TIMER_NOT_EXITED=2, CS_TIMER_NOT_READY=3,
    CS_TIMER_NO_RSDP=4, CS_TIMER_ACPI=5, CS_TIMER_UNSUPPORTED=6, CS_TIMER_VALUE=7,
    CS_TIMER_STALLED=8 };
enum { CS_TIMER_TRACE_MAGIC=0x31545043, CS_TIMER_TRACE_VERSION=1, CS_TIMER_TRACE_BYTES=48,
    CS_TIMER_POLLS=20000000, CS_TIMER_TARGET_TICKS=3580 };
typedef uint32_t (*cs_native_port32)(void *context,uint16_t port);
/* Physical reads are allowed only inside one validated map descriptor of type
   0, 6, 9 or 10 and inside [window_physical, window_physical+window_size). The
   window maps physical P to integer address window_base+(P-window_physical). */
typedef struct {
    const unsigned char *map; size_t map_size,map_stride; uint32_t map_version;
    uint64_t window_physical,window_size; uintptr_t window_base;
} cs_native_memory;
int cs_native_read(void *memory,uint64_t physical,unsigned char *out,size_t length);
uint32_t cs_native_timer_probe(const cs_uefi_handoff *handoff,uint32_t descriptors_ready,
    const cs_native_memory *memory,cs_native_port32 port,void *port_context,unsigned char *trace);
#endif
