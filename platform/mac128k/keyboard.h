/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MAC128K_KEYBOARD_H
#define CLASSICK_MAC128K_KEYBOARD_H
#include "rom.h"
#include "../../core/input/m0110-reply.h"

typedef struct {
    uint32_t phase, started;
} cs_mac_keyboard;
enum { CS_MAC_KEY_WAIT=0, CS_MAC_KEY_EVENT=1, CS_MAC_KEY_EMPTY=2,
       CS_MAC_KEY_INVALID=3, CS_MAC_KEY_TIMEOUT=4 };
/* Sole owner of VIA SR and ACR shift bits; shift interrupt must be disabled.
 * tick is a monotonic, wrapping VBL counter. Each call returns immediately. */
void cs_mac_keyboard_begin(cs_mac_keyboard *k);
int cs_mac_keyboard_step(cs_mac_keyboard *k, const cs_mac_io *io,
                         uint32_t tick, cs_m0110_event *out);
#endif
