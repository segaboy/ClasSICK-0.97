/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_KEYBOARD_CONFIG_H
#define CLASSICK_PC286_KEYBOARD_CONFIG_H
#include <stdint.h>

enum { CS_286_CFG_WAIT=0, CS_286_CFG_SEND=1, CS_286_CFG_READ=2,
       CS_286_CFG_READY=3, CS_286_CFG_PENDING=4, CS_286_CFG_STATUS=5,
       CS_286_CFG_TIMEOUT=6, CS_286_CFG_INCOMPATIBLE=7, CS_286_CFG_STATE=8 };
typedef struct { unsigned phase,polls,limit,result; uint8_t value; } cs_pc286_keyboard_config;
/* Bounded configuration query that does not change the configuration.
 * Caller must own a quiescent
 * controller and keep IRQ1 masked until completion. step never performs I/O:
 * SEND requests command 20h on port 64h, READ requests one byte on port 60h.
 * Unexpected pending output is refused without a read. A racing input byte
 * cannot be distinguished from the requested response by this protocol. */
int cs_pc286_keyboard_config_begin(cs_pc286_keyboard_config *s,unsigned limit);
int cs_pc286_keyboard_config_step(cs_pc286_keyboard_config *s,uint8_t status);
int cs_pc286_keyboard_config_reply(cs_pc286_keyboard_config *s,uint8_t value);
#endif
