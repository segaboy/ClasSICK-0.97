/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MEMORY_GUEST_H
#define CLASSICK_MEMORY_GUEST_H
#include <stdint.h>
#include <stddef.h>
#define CS_GUEST_READ 1u
#define CS_GUEST_WRITE 2u
#define CS_GUEST_EXECUTE 4u
#define CS_GUEST_MAX_REGIONS 8u
typedef enum { CS_GUEST_OK=0,CS_GUEST_ARGUMENT=1,CS_GUEST_MAP=2,
    CS_GUEST_WIDTH=3,CS_GUEST_ADDRESS=4,CS_GUEST_PERMISSION=5 } cs_guest_result;
typedef struct {
    const uint8_t *bytes; uint8_t *writable; uint32_t base,size,flags;
} cs_guest_region;
typedef struct { const cs_guest_region *regions; size_t count; } cs_guest_map;
typedef struct { const uint8_t *bytes; uint8_t *writable; } cs_guest_view;
/* SPEC-0022: live, truthful, synchronized buffers; outputs/objects disjoint.
 * Window resolves arbitrary byte spans; read/write impose width alignment.
 * A view is valid only while its borrowed map and buffers remain unchanged. */
cs_guest_result cs_guest_validate(const cs_guest_map *m);
cs_guest_result cs_guest_window(const cs_guest_map *m,uint32_t at,uint32_t bytes,
    uint32_t access,cs_guest_view *out);
cs_guest_result cs_guest_read(const cs_guest_map *m,uint32_t at,unsigned width,
    uint32_t access,uint32_t *out);
cs_guest_result cs_guest_write(const cs_guest_map *m,uint32_t at,unsigned width,uint32_t value);
#endif
