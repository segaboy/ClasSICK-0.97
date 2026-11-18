/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Independently authored modeled transport; SPEC-0114.
 * The symbol alphabet is a wire-format fact observed in the admitted original
 * BootHush fixture. No emulator or fixture decoder implementation is imported. */
#include "iwm-read.h"

#define IWM_BOUND UINT32_C(0x494f5231)
extern int cs_mac_iwm_capture(uint8_t *address, uint8_t *wire,
    const uint8_t *expected);

static const uint8_t symbols[64] = {
    0x96,0x97,0x9a,0x9b,0x9d,0x9e,0x9f,0xa6,
    0xa7,0xab,0xac,0xad,0xae,0xaf,0xb2,0xb3,
    0xb4,0xb5,0xb6,0xb7,0xb9,0xba,0xbb,0xbc,
    0xbd,0xbe,0xbf,0xcb,0xcd,0xce,0xcf,0xd3,
    0xd6,0xd7,0xd9,0xda,0xdb,0xdc,0xdd,0xde,
    0xdf,0xe5,0xe6,0xe7,0xe9,0xea,0xeb,0xec,
    0xed,0xee,0xef,0xf2,0xf3,0xf4,0xf5,0xf6,
    0xf7,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};

static int unsymbol(uint8_t value)
{
    unsigned i;
    for (i = 0; i < 64u; ++i) if (symbols[i] == value) return (int)i;
    return -1;
}

static int unpack(const uint8_t *input, unsigned count, uint8_t *bytes)
{
    int top = unsymbol(input[0]);
    unsigned i;
    if (top < 0) return 0;
    for (i = 0; i < count; ++i) {
        int low = unsymbol(input[i + 1u]);
        unsigned shift = 4u - 2u * i;
        if (low < 0) return 0;
        bytes[i] = (uint8_t)((((unsigned)top >> shift) & 3u) * 64u + (unsigned)low);
    }
    return 1;
}

cs_mac_iwm_error cs_mac_iwm_decode(uint16_t track, uint16_t sector,
    const uint8_t *address, const uint8_t *wire, uint8_t *decoded)
{
    unsigned i, position = 1u, produced = 0u;
    uint8_t sums[3] = {0, 0, 0};
    uint8_t check[3];
    if (address == NULL || wire == NULL || decoded == NULL || track >= 80u ||
        sector >= 12u - track / 16u) return CS_MAC_IWM_ARGUMENT;
    if (address[0] != symbols[track & 63u] || address[1] != symbols[sector] ||
        address[2] != symbols[track >> 6] || address[3] != symbols[2] ||
        address[4] != symbols[(track & 63u) ^ sector ^ (track >> 6) ^ 2u] ||
        address[5] != 0xdeu || address[6] != 0xaau) return CS_MAC_IWM_ADDRESS;
    if (wire[0] != symbols[sector] || wire[704] != 0xdeu || wire[705] != 0xaau)
        return CS_MAC_IWM_DATA;
    while (produced < CS_MAC_IWM_DECODED_BYTES) {
        uint8_t encoded[3];
        unsigned count = CS_MAC_IWM_DECODED_BYTES - produced;
        unsigned carry = (unsigned)sums[2] >> 7;
        if (count > 3u) count = 3u;
        sums[2] = (uint8_t)(((unsigned)sums[2] << 1) | carry);
        if (!unpack(wire + position, count, encoded)) return CS_MAC_IWM_SYMBOL;
        position += count + 1u;
        for (i = 0; i < count; ++i) {
            unsigned previous = i == 0u ? 2u : i - 1u;
            uint8_t value = (uint8_t)(encoded[i] ^ sums[previous]);
            unsigned total = (unsigned)sums[i] + value + carry;
            decoded[produced++] = value;
            sums[i] = (uint8_t)total;
            carry = total >> 8;
        }
    }
    if (position != 700u || !unpack(wire + position, 3u, check))
        return CS_MAC_IWM_SYMBOL;
    for (i = 0; i < 3u; ++i) if (check[i] != sums[i]) return CS_MAC_IWM_CHECKSUM;
    return CS_MAC_IWM_OK;
}

static void touch(unsigned selector)
{
    volatile uint8_t *port = (volatile uint8_t *)(uintptr_t)
        (UINT32_C(0xc00000) + (uint32_t)selector * UINT32_C(512));
    (void)*port;
}

static void command(unsigned value)
{
    touch((value & 1u) != 0u ? 1u : 0u);
    touch((value & 2u) != 0u ? 3u : 2u);
    touch((value & 4u) != 0u ? 5u : 4u);
    touch(7u);
    touch(6u);
}

int cs_mac_iwm_bind(cs_mac_iwm *provider, cs_block_device *device)
{
    if (provider == NULL || device == NULL) return 0;
    provider->bound = IWM_BOUND;
    provider->reads = 0;
    provider->track = 0;
    provider->initialized = 0;
    provider->error = CS_MAC_IWM_OK;
    device->blocks = CS_MAC_IWM_BLOCKS;
    device->read = cs_mac_iwm_read_one;
    device->context = provider;
    return 1;
}

cs_block_io_result cs_mac_iwm_read_one(void *context, uint32_t lba, uint8_t *block)
{
    cs_mac_iwm *provider = (cs_mac_iwm *)context;
    uint16_t target = 0, sector;
    unsigned i;
    uint8_t expected[3];
    uint32_t relative = lba;
    if (provider == NULL || block == NULL || provider->bound != IWM_BOUND ||
        lba >= CS_MAC_IWM_BLOCKS || provider->track >= 80u ||
        block == provider->decoded) return CS_BLOCK_IO_ERROR;
    while (relative >= (uint32_t)(12u - target / 16u)) {
        relative -= (uint32_t)(12u - target / 16u);
        ++target;
    }
    sector = (uint16_t)relative;
    provider->error = CS_MAC_IWM_TIMEOUT;
    if (provider->initialized == 0u) {
        volatile uint8_t *pa = (volatile uint8_t *)(uintptr_t)UINT32_C(0xeffffe);
        volatile uint8_t *ddra = (volatile uint8_t *)(uintptr_t)UINT32_C(0xefe7fe);
        *pa = (uint8_t)(*pa & 0xdfu);
        *ddra = (uint8_t)(*ddra | 0x20u);
        touch(9u); touch(10u); touch(14u); touch(12u);
        command(4u); /* Explicit modeled outward direction. */
        for (i = 0; i < 80u; ++i) command(1u);
        provider->track = 0;
        provider->initialized = 1;
    }
    command(2u);
    command(target < provider->track ? 4u : 0u);
    while (provider->track != target) {
        command(1u);
        if (provider->track < target) ++provider->track;
        else --provider->track;
    }
    touch(14u); touch(12u);
    expected[0] = symbols[target & 63u];
    expected[1] = symbols[sector];
    expected[2] = symbols[target >> 6];
    if (!cs_mac_iwm_capture(provider->address, provider->wire, expected))
        return CS_BLOCK_IO_ERROR;
    provider->error = cs_mac_iwm_decode(target, sector, provider->address,
        provider->wire, provider->decoded);
    if (provider->error != CS_MAC_IWM_OK) return CS_BLOCK_IO_ERROR;
    for (i = 0; i < CS_BLOCK_BYTES; ++i) block[i] = provider->decoded[i + 12u];
    ++provider->reads;
    return CS_BLOCK_IO_OK;
}
