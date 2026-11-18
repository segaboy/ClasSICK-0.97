/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MAC128K_IWM_READ_H
#define CLASSICK_MAC128K_IWM_READ_H

#include "../../core/storage/block-read.h"

#define CS_MAC_IWM_BLOCKS 800u
#define CS_MAC_IWM_WIRE_BYTES 706u
#define CS_MAC_IWM_DECODED_BYTES 524u

typedef enum {
    CS_MAC_IWM_OK = 0,
    CS_MAC_IWM_ARGUMENT = 1,
    CS_MAC_IWM_TIMEOUT = 2,
    CS_MAC_IWM_ADDRESS = 3,
    CS_MAC_IWM_SYMBOL = 4,
    CS_MAC_IWM_DATA = 5,
    CS_MAC_IWM_CHECKSUM = 6
} cs_mac_iwm_error;

/* Exclusive owner of the modeled internal drive and VIA PA5. Interrupts must
 * remain masked during read. Caller supplies stable, disjoint native objects.
 * Only current BootHush behavior is admitted; no physical timing claim. */
typedef struct {
    uint32_t bound;
    uint32_t reads;
    uint16_t track;
    uint16_t initialized;
    cs_mac_iwm_error error;
    uint8_t address[7];
    uint8_t wire[CS_MAC_IWM_WIRE_BYTES];
    uint8_t decoded[CS_MAC_IWM_DECODED_BYTES];
} cs_mac_iwm;

/* Binding does not access hardware; refusal preserves both destinations. */
int cs_mac_iwm_bind(cs_mac_iwm *provider, cs_block_device *device);
cs_block_io_result cs_mac_iwm_read_one(void *context, uint32_t lba,
    uint8_t *block);

/* Model-wire validator. On failure decoded may contain partial bytes. This
 * helper is not the block publication boundary and retains no pointer. */
cs_mac_iwm_error cs_mac_iwm_decode(uint16_t track, uint16_t sector,
    const uint8_t *address, const uint8_t *wire, uint8_t *decoded);

#endif
