/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SERVICES_TRAPS_H
#define CLASSICK_SERVICES_TRAPS_H
#include <stdint.h>
enum { CS_TRAP_ENTRIES=512, CS_TRAP_GET=0x46, CS_TRAP_SET=0x47 };
typedef enum {
    CS_TRAP_OK=0, CS_TRAP_ARGUMENT=1, CS_TRAP_STATE=2, CS_TRAP_NOT_TRAP=3,
    CS_TRAP_NUMBER=4, CS_TRAP_ADDRESS=5, CS_TRAP_UNSUPPORTED_FORMAT=6,
    CS_TRAP_UNSUPPORTED_FLAGS=7, CS_TRAP_UNIMPLEMENTED=8, CS_TRAP_NEEDS_EXECUTION=9
} cs_trap_result;
typedef struct { uint16_t word,number,toolbox,a0_result,auto_pop,reserved,flags; } cs_trap_word;
typedef struct {
    uint32_t rom_base,ram_begin,ram_end,get_token,set_token,unsupported_token;
} cs_trap_config;
typedef struct { uint32_t magic; cs_trap_config config; uint32_t entries[CS_TRAP_ENTRIES]; } cs_trap_gateway;
typedef struct { uint32_t d[8],a[8],pc; uint16_t sr; } cs_trap_frame;
typedef struct { cs_trap_result status; cs_trap_word decoded; uint32_t target,pc; } cs_trap_report;
/* Live aligned nonoverlapping objects, synchronized by caller; SPEC-0020. */
cs_trap_result cs_trap_decode(uint16_t word,cs_trap_word *out);
cs_trap_result cs_trap_init(cs_trap_gateway *out,const cs_trap_config *config);
cs_trap_result cs_trap_get(const cs_trap_gateway *gateway,uint16_t number,uint32_t *out);
cs_trap_result cs_trap_set(cs_trap_gateway *gateway,uint16_t number,uint32_t address);
cs_trap_result cs_trap_call(cs_trap_gateway *gateway,uint16_t word,cs_trap_frame *frame,cs_trap_report *report);
cs_trap_result cs_trap_direct(cs_trap_gateway *gateway,uint32_t target,cs_trap_frame *frame,cs_trap_report *report);
#endif
