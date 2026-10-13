/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MEMORY_MANAGER_H
#define CLASSICK_MEMORY_MANAGER_H
#include "guest.h"
#define CS_MM_BLOCKS 32u
#define CS_MM_CELL_BYTES (CS_MM_BLOCKS*4u)
#define CS_MM_CLEAR 1u
#define CS_MM_SYSTEM 2u
#define CS_MM_LOCK UINT32_C(0x80000000)
enum { CS_MM_NO_ERROR=0,CS_MM_FULL=-108,CS_MM_NIL_HANDLE=-109,CS_MM_WRONG_ZONE=-111 };
typedef enum { CS_MM_OK=0,CS_MM_ARGUMENT=1,CS_MM_STATE=2,CS_MM_MEMORY=3,
    CS_MM_OPERATION=4,CS_MM_FLAGS=5,CS_MM_INTERRUPT=6 } cs_mm_result;
typedef enum { CS_MM_NEW_PTR=0,CS_MM_DISPOS_PTR=1,CS_MM_GET_PTR_SIZE=2,CS_MM_SET_PTR_SIZE=3,
    CS_MM_NEW_HANDLE=4,CS_MM_DISPOS_HANDLE=5,CS_MM_GET_HANDLE_SIZE=6,CS_MM_SET_HANDLE_SIZE=7,
    CS_MM_HLOCK=8,CS_MM_HUNLOCK=9,CS_MM_BLOCK_MOVE=10 } cs_mm_operation;
typedef struct { uint32_t identity,begin,end; } cs_mm_zone;
typedef struct { cs_mm_zone system,application; } cs_mm_config;
typedef struct { uint32_t kind,zone,address,size,extent; } cs_mm_block;
typedef struct {
    uint32_t magic;const cs_guest_map *map;cs_mm_config config;cs_mm_block blocks[CS_MM_BLOCKS];
} cs_mm_manager;
typedef struct {
    cs_mm_operation operation;uint32_t address,other,size;unsigned flags,interrupt_context;
} cs_mm_request;
typedef struct { int16_t code;uint32_t value,moved; } cs_mm_reply;
/* SPEC-0023: caller owns synchronized disjoint live objects and truthful map.
 * API errors use an explicit reply; contract errors preserve reply/state/RAM. */
cs_mm_result cs_mm_init(cs_mm_manager *out,const cs_guest_map *map,const cs_mm_config *config);
cs_mm_result cs_mm_execute(cs_mm_manager *m,const cs_mm_request *request,cs_mm_reply *out);
#endif
