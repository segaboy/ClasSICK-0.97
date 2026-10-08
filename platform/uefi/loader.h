/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_UEFI_LOADER_H
#define CLASSICK_UEFI_LOADER_H
#include "abi.h"
#include "contract.h"
enum { CS_LOADER_BUNDLE=524288, CS_LOADER_MAP_OFFSET=4096,
    CS_LOADER_MAP_BYTES=262144, CS_LOADER_STACK_OFFSET=266240,
    CS_LOADER_STACK_BYTES=65536, CS_LOADER_FAULT_OFFSET=331776,
    CS_LOADER_ARENA_OFFSET=348160, CS_LOADER_ARENA_BYTES=131072,
    CS_LOADER_TRACE_OFFSET=479232, CS_LOADER_TRACE_BYTES=16384 };
typedef struct {
    uint64_t magic; uint32_t version,size,stage,reason;
    uint64_t firmware_status,image_base,image_size,bundle_base,bundle_size;
    cs_uefi_framebuffer framebuffer;
    uint64_t map_base,map_size,map_stride,map_key,stack_top,fault_top;
    uint64_t arena_base,arena_size,trace_base,trace_size;
    uint32_t map_version,exit_attempts,span_count,native_entered;
    cs_uefi_owned_span spans[8];
} cs_uefi_handoff;
typedef struct { cs_efi_status status; uint32_t attempts,exited; cs_uefi_handoff *handoff; } cs_uefi_load_result;
_Static_assert(sizeof(cs_uefi_handoff)<=4096,"Handoff fits reserved header page");
/* Caller supplies truthful live/disjoint metadata; no native jump here. */
/* Callers supply live readable bytes; checksum bytes 16..19 are treated as zero.
   For size zero no bytes are read. Output records are live, aligned and disjoint
   from input/firmware storage. Image spans has room for seven output records. */
uint32_t cs_uefi_table_crc(const void *table,size_t size);
cs_uefi_result cs_uefi_image_spans(const unsigned char *map,size_t length,size_t stride,
    uint32_t version,uint64_t image_base,uint64_t image_size,cs_uefi_owned_span *out,size_t *count);
cs_efi_status cs_uefi_loader_run(void *image,cs_efi_system *system,cs_uefi_load_result *out);
#endif
