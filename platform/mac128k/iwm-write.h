/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MAC128K_IWM_WRITE_H
#define CLASSICK_MAC128K_IWM_WRITE_H
#include "iwm-read.h"

typedef enum {
    CS_MAC_IWM_WRITE_OK=0,
    CS_MAC_IWM_WRITE_ARGUMENT=1,
    CS_MAC_IWM_WRITE_READ=2,
    CS_MAC_IWM_WRITE_TIMEOUT=3,
    CS_MAC_IWM_WRITE_REJECTED=4
} cs_mac_iwm_write_result;

/* Original model-wire encoder. All live spans are disjoint: tags[12],
 * block[512], wire[706]. Invalid sector/null arguments preserve wire.
 * The caller supplies the sector number within the selected track. */
int cs_mac_iwm_encode_sector(uint16_t sector,const uint8_t *tags,
    const uint8_t *block,uint8_t *wire);

/* SPEC-0125: one modeled sector write, preserving existing twelve tag bytes.
 * Borrow an exclusively owned bound reader and scratch[512], both disjoint
 * from block[512] and all controls. IRQs masked; truthful stable native spans.
 * The original sector is first acquired through the validated read provider.
 * This is not a multi-sector transaction or physical hardware contract.
 * A transport failure after the final byte can have an uncertain commit;
 * callers must re-read rather than assume failure rolled back the medium.
 * Explicit emulator writable-copy admission and successful export are separate.
 */
cs_mac_iwm_write_result cs_mac_iwm_write_one(cs_mac_iwm *provider,uint32_t lba,
    const uint8_t *block,uint8_t *scratch);

/* Native transport of exactly wire[706], adding D5 AA AD. Returns1 on final
 * acknowledgement,0 on bounded poll timeout,-1 on modeled rejection. */
int cs_mac_iwm_send(const uint8_t *wire);
#endif
