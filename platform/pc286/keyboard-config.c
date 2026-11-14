/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "keyboard-config.h"

int cs_pc286_keyboard_config_begin(cs_pc286_keyboard_config *s,unsigned limit)
{
    if(!s || !limit)return CS_286_CFG_STATE;
    s->phase=0;s->polls=0;s->limit=limit;s->result=CS_286_CFG_WAIT;s->value=0;
    return CS_286_CFG_WAIT;
}

int cs_pc286_keyboard_config_step(cs_pc286_keyboard_config *s,uint8_t status)
{
    if(!s || !s->limit || s->phase>3u || s->result>CS_286_CFG_STATE ||
       (s->phase<3u && s->result!=CS_286_CFG_WAIT) ||
       (s->phase==3u && s->result<CS_286_CFG_READY))return CS_286_CFG_STATE;
    if(s->phase==3u)return (int)s->result;
    if(s->phase==2u) {s->phase=3;s->result=CS_286_CFG_STATE;return CS_286_CFG_STATE;}
    if(s->polls>=s->limit) {s->phase=3;s->result=CS_286_CFG_TIMEOUT;return CS_286_CFG_TIMEOUT;}
    ++s->polls;
    if(status&0xc0u) {s->phase=3;s->result=CS_286_CFG_STATUS;return CS_286_CFG_STATUS;}
    if(s->phase==0u) {
        if(status&1u) {s->phase=3;s->result=CS_286_CFG_PENDING;return CS_286_CFG_PENDING;}
        if(status&2u)return CS_286_CFG_WAIT;
        s->phase=1;s->polls=0;return CS_286_CFG_SEND;
    }
    if(!(status&1u))return CS_286_CFG_WAIT;
    if(status&0x20u) {s->phase=3;s->result=CS_286_CFG_STATUS;return CS_286_CFG_STATUS;}
    s->phase=2;return CS_286_CFG_READ;
}

int cs_pc286_keyboard_config_reply(cs_pc286_keyboard_config *s,uint8_t value)
{
    if(!s || !s->limit || s->phase!=2u || s->result!=CS_286_CFG_WAIT)return CS_286_CFG_STATE;
    s->value=value;s->phase=3;
    s->result=(value&0x50u)==0x40u?CS_286_CFG_READY:CS_286_CFG_INCOMPATIBLE;
    return (int)s->result;
}
