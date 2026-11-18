/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../platform/mac128k/iwm-read.h"

#ifndef PROBE_CASE
#define PROBE_CASE 0
#endif
#ifndef EXPECT_TAGS
#define EXPECT_TAGS 0
#endif

volatile uint32_t cs_mac_diag[7];
static cs_mac_iwm provider;
static cs_block_device device;
static uint8_t output[1026];
static uint8_t staging[1026];

static uint8_t pattern(uint32_t lba, unsigned offset)
{
    return (uint8_t)(lba * 37u + offset * 29u + (offset >> 3));
}

static int check_bytes(const uint8_t *bytes, uint32_t lba)
{
    unsigned i;
    for (i = 0; i < 512u; ++i) if (bytes[i] != pattern(lba, i)) return 0;
    return 1;
}

static int guards(void)
{
    return output[0] == 0xacu && output[1025] == 0xacu &&
        staging[0] == 0xbdu && staging[1025] == 0xbdu;
}

static int untouched(void)
{
    unsigned i;
    for (i = 1; i <= 1024u; ++i) if (output[i] != 0xacu) return 0;
    return guards();
}

#if PROBE_CASE == 0
static int malformed_controls(void)
{
    static const unsigned positions[] = {0,1,2,3,4,5,6};
    unsigned i;
    for (i = 0; i < 7u; ++i) {
        uint8_t old = provider.address[positions[i]];
        provider.address[positions[i]] = 0;
        if (cs_mac_iwm_decode(0, 2, provider.address, provider.wire, provider.decoded)
            != CS_MAC_IWM_ADDRESS) return 0;
        provider.address[positions[i]] = old;
    }
    for (i = 0; i < 3u; ++i) {
        unsigned position = i == 0u ? 0u : 703u + i;
        uint8_t old = provider.wire[position];
        provider.wire[position] = 0;
        if (cs_mac_iwm_decode(0, 2, provider.address, provider.wire, provider.decoded)
            != CS_MAC_IWM_DATA) return 0;
        provider.wire[position] = old;
    }
    {
        uint8_t old = provider.wire[1];
        provider.wire[1] = 0;
        if (cs_mac_iwm_decode(0, 2, provider.address, provider.wire, provider.decoded)
            != CS_MAC_IWM_SYMBOL) return 0;
        provider.wire[1] = old;
    }
    {
        uint8_t old = provider.wire[701];
        provider.wire[701] = old == 0x96u ? 0x97u : 0x96u;
        if (cs_mac_iwm_decode(0, 2, provider.address, provider.wire, provider.decoded)
            != CS_MAC_IWM_CHECKSUM) return 0;
        provider.wire[701] = old;
    }
    return cs_mac_iwm_decode(0, 2, provider.address, provider.wire, provider.decoded)
        == CS_MAC_IWM_OK;
}
#endif

static int probe(void)
{
    unsigned i;
    cs_block_result result;
    for (i = 0; i < 1026u; ++i) { output[i] = 0xacu; staging[i] = 0xbdu; }
    if (!cs_mac_iwm_bind(&provider, &device)) return 0;
    cs_mac_diag[1] = 1;
    if (cs_block_read(&device, 800u, 1, output + 1, 1024u, staging + 1, 1024u)
        != CS_BLOCK_RANGE || !untouched() || provider.initialized != 0u) return 0;
    if (cs_mac_iwm_read_one(&provider, 800u, staging + 1) != CS_BLOCK_IO_ERROR ||
        provider.initialized != 0u) return 0;
#if PROBE_CASE == 0
    {
        static const uint16_t lbas[] = {0,11,12,191,192,367,368,527,528,671,672,799,2,2};
        cs_mac_diag[1] = 2;
        result = cs_block_read(&device, 191u, 2, output + 1, 1024u, staging + 1, 1024u);
        cs_mac_diag[2] = (uint32_t)result;
        if (result != CS_BLOCK_OK || !check_bytes(output + 1, 191u) ||
            !check_bytes(output + 513, 192u) || !guards()) return 0;
        for (i = 1; i <= 1024u; ++i) output[i] = 0xacu;
        for (i = 0; i < sizeof(lbas) / sizeof(lbas[0]); ++i) {
            unsigned n;
            cs_mac_diag[1] = 10u + i;
            result = cs_block_read(&device, lbas[i], 1, output + 1, 1024u, staging + 1, 1024u);
            cs_mac_diag[2] = (uint32_t)result;
            if (result != CS_BLOCK_OK || !check_bytes(output + 1, lbas[i]) || !guards()) return 0;
            for (n = 513u; n <= 1024u; ++n) if (output[n] != 0xacu) return 0;
            for (n = 0; n < 12u; ++n) {
                uint8_t expected = EXPECT_TAGS ? (uint8_t)((uint32_t)lbas[i] * 11u + n * 7u + 3u) : 0u;
                if (provider.decoded[n] != expected) return 0;
            }
        }
        cs_mac_diag[1] = 30;
        if (!malformed_controls()) return 0;
    }
#else
    cs_mac_diag[1] = 40;
    result = cs_block_read(&device, 0, PROBE_CASE == 1 ? 2u : 1u,
        output + 1, 1024u, staging + 1, 1024u);
    cs_mac_diag[2] = (uint32_t)result;
    if (result != CS_BLOCK_IO || !untouched()) return 0;
    if (PROBE_CASE == 1 && (provider.reads != 1u || provider.error != CS_MAC_IWM_CHECKSUM ||
        !check_bytes(staging + 1, 0))) return 0;
    if (PROBE_CASE == 2 && (provider.reads != 0u || provider.error != CS_MAC_IWM_TIMEOUT)) return 0;
#endif
    return 1;
}

void cs_mac_main(void)
{
    int passed = probe();
    cs_mac_diag[3] = (uint32_t)provider.error;
    cs_mac_diag[4] = provider.reads;
    cs_mac_diag[5] = provider.track;
    cs_mac_diag[0] = passed ? UINT32_C(0xb7) : UINT32_C(0xf7);
    for (;;) { }
}

void cs_mac_line_a_handler(void)
{
    cs_mac_diag[6] = UINT32_C(0xf002);
    for (;;) { }
}
