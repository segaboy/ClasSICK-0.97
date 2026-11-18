/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CORE_STORAGE_MAC_BOOT_HEADER_H
#define CLASSICK_CORE_STORAGE_MAC_BOOT_HEADER_H
#include "block-read.h"

#define CS_MAC_BOOT_AREA_BYTES 1024u
#define CS_MAC_BOOT_NAME_COUNT 7u
enum {
    CS_MAC_BOOT_SYSTEM=0, CS_MAC_BOOT_SHELL=1, CS_MAC_BOOT_DEBUGGER1=2,
    CS_MAC_BOOT_DEBUGGER2=3, CS_MAC_BOOT_SCREEN=4, CS_MAC_BOOT_STARTUP=5,
    CS_MAC_BOOT_SCRAP=6
};
typedef enum {
    CS_MAC_BOOT_OK=0, CS_MAC_BOOT_ARGUMENT=1, CS_MAC_BOOT_RANGE=2,
    CS_MAC_BOOT_SIGNATURE=3, CS_MAC_BOOT_NAME=4, CS_MAC_BOOT_UNAVAILABLE=5,
    CS_MAC_BOOT_IO=6, CS_MAC_BOOT_PROVIDER=7
} cs_mac_boot_result;
typedef struct { uint8_t length,bytes[15]; } cs_mac_boot_name;
typedef struct {
    uint32_t heap_128k,heap_256k,heap_other;
    uint16_t signature,version,page_flags,fcb_count,event_count;
    cs_mac_boot_name names[CS_MAC_BOOT_NAME_COUNT];
} cs_mac_boot_header;
typedef struct {
    uint8_t area[CS_MAC_BOOT_AREA_BYTES],staging[CS_MAC_BOOT_AREA_BYTES];
} cs_mac_boot_workspace;

/* SPEC-0119. Data only: bytes2..5 and138..1023 are never decoded/exported.
 * Exact1024-byte input; every refusal preserves the complete out object.
 * Truthful live disjoint stable native objects within single native segments
 * and a conforming exclusive provider are caller preconditions. No far-alias
 * or arbitrary overlap detection, guest pointers, allocation or reentry.
 * Names are copied raw counts/bytes with zeroed unused tails, not C strings.
 * Numeric fields are raw; no version dispatch or startup ABI is inferred. */
cs_mac_boot_result cs_mac_boot_header_decode(const uint8_t *area,size_t length,
    cs_mac_boot_header *out);
/* Only LBA0..1, atomically acquired through SPEC-0111. On provider failure,
 * area and out are unchanged; staging/provider partial effects may remain. */
cs_mac_boot_result cs_mac_boot_header_read(const cs_block_device *device,
    cs_mac_boot_workspace *scratch,cs_mac_boot_header *out);
#endif
