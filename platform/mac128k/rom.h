/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MAC128K_ROM_H
#define CLASSICK_MAC128K_ROM_H
#include <stdint.h>
#include <stddef.h>
#include "../../core/graphics/surface.h"
enum { CS_MAC_ROM_BYTES=65536, CS_MAC_SCREEN_BYTES=21888,
    CS_MAC_TOTAL_TICKS=3610, CS_MAC_SEAL_OFFSET=256 };
typedef struct {
    void *context;
    uint8_t (*read)(void *context, unsigned reg);
    void (*write)(void *context, unsigned reg, uint8_t value);
} cs_mac_io;
typedef struct {
    uint32_t ticks, presses, synchronized, mouse_down;
} cs_mac_loop;
typedef struct {
    uint32_t magic, stage, ticks, presses, traps, image_crc, fault;
} cs_mac_diagnostics;
extern volatile cs_mac_diagnostics cs_mac_diag;
uint32_t cs_mac_image_crc(const volatile uint8_t *image, size_t size);
void cs_mac_via_init(const cs_mac_io *io);
void cs_mac_loop_begin(cs_mac_loop *loop, uint8_t mouse);
/* Returns 1 when visible state changed. VBL is acknowledged only through reg 1. */
int cs_mac_loop_poll(cs_mac_loop *loop, const cs_mac_io *io);
int cs_mac_scene(const cs_surface *surface, uint32_t ticks, uint32_t presses);
#endif
