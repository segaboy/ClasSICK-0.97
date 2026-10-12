/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "rom.h"

uint32_t cs_mac_image_crc(const volatile uint8_t *image, size_t size)
{
    uint32_t crc = UINT32_MAX;
    for (size_t i=0; i<size; ++i) {
        uint8_t byte = i>=CS_MAC_SEAL_OFFSET && i<CS_MAC_SEAL_OFFSET+4u ? 0 : image[i];
        crc ^= byte;
        for (unsigned bit=0; bit<8u; ++bit)
            crc = (crc >> 1u) ^ ((crc & 1u) != 0u ? UINT32_C(0xEDB88320) : 0u);
    }
    return crc ^ UINT32_MAX;
}

void cs_mac_via_init(const cs_mac_io *io)
{
    io->write(io->context, 14, 0x7Fu);
    io->write(io->context, 11, 0);
    io->write(io->context, 12, 0);
    io->write(io->context, 0, 0x80u);
    io->write(io->context, 2, 0x80u);
    (void)io->read(io->context, 1);
}

void cs_mac_loop_begin(cs_mac_loop *loop, uint8_t mouse)
{
    loop->ticks=0; loop->presses=0; loop->synchronized=0;
    loop->mouse_down=(mouse & 8u)==0u ? 1u : 0u;
}

int cs_mac_loop_poll(cs_mac_loop *loop, const cs_mac_io *io)
{
    int changed=0;
    uint32_t down=(io->read(io->context,0) & 8u)==0u ? 1u : 0u;
    if (down!=0u && loop->mouse_down==0u) {
        if (loop->presses!=UINT32_MAX) ++loop->presses;
        changed=1;
    }
    loop->mouse_down=down;
    if ((io->read(io->context,13) & 2u)!=0u) {
        (void)io->read(io->context,1);
        if (loop->synchronized==0u) loop->synchronized=1;
        else if (loop->ticks<CS_MAC_TOTAL_TICKS) {
            ++loop->ticks;
            if (loop->ticks % 60u == 0u || loop->ticks==CS_MAC_TOTAL_TICKS) changed=1;
        }
    }
    return changed;
}
