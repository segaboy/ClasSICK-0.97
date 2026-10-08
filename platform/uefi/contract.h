/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_UEFI_CONTRACT_H
#define CLASSICK_UEFI_CONTRACT_H
#include <stddef.h>
#include <stdint.h>

typedef enum { CS_UEFI_OK=0, CS_UEFI_ARGUMENT=1, CS_UEFI_FORMAT=2,
    CS_UEFI_LIMIT=3, CS_UEFI_OVERFLOW=4, CS_UEFI_OVERLAP=5,
    CS_UEFI_OWNERSHIP=6, CS_UEFI_STATE=7 } cs_uefi_result;
typedef struct { uint64_t base,size; uint32_t width,height,pitch,format; } cs_uefi_framebuffer;
typedef struct { uint64_t stride,required; } cs_uefi_framebuffer_layout;
typedef struct { uint64_t base,size; uint32_t kind; } cs_uefi_owned_span;
typedef struct { uint32_t phase,attempts; uint64_t key; } cs_uefi_exit_state;
enum { CS_UEFI_READY=0, CS_UEFI_SNAPSHOT=1, CS_UEFI_RETRY=2,
    CS_UEFI_EXITED=3, CS_UEFI_FAILED=4 };
enum { CS_UEFI_OTHER_BOOT=0, CS_UEFI_GET_MAP=1, CS_UEFI_EXIT_BOOT=2,
    CS_UEFI_NATIVE_LOOP=3 };

/* Truthful live/disjoint native storage; physical values are never dereferenced. */
cs_uefi_result cs_uefi_check_framebuffer(const cs_uefi_framebuffer *input,
    cs_uefi_framebuffer_layout *out);
cs_uefi_result cs_uefi_check_map(const unsigned char *bytes,size_t length,
    size_t stride,uint32_t version,size_t *out_count);
cs_uefi_result cs_uefi_check_owned(const unsigned char *bytes,size_t length,
    size_t stride,uint32_t version,const cs_uefi_owned_span *spans,size_t count,
    const cs_uefi_framebuffer *framebuffer);
cs_uefi_result cs_uefi_exit_init(cs_uefi_exit_state *state);
cs_uefi_result cs_uefi_exit_snapshot(cs_uefi_exit_state *state,uint64_t key);
/* outcome: success 0, stale map key 1, other failure 2. */
cs_uefi_result cs_uefi_exit_observe(cs_uefi_exit_state *state,uint32_t outcome);
int cs_uefi_exit_allowed(const cs_uefi_exit_state *state,uint32_t call);
#endif
