/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "keyboard.h"

enum { IDLE=0, OUT=1, IN=2, FAILED=3, SHIFT=10, ACR=11, IFR=13 };
static void mode(const cs_mac_io *io, uint8_t bits)
{
    uint8_t old=io->read(io->context,ACR);
    io->write(io->context,ACR,(uint8_t)((old & 0xe3u)|bits));
}
void cs_mac_keyboard_begin(cs_mac_keyboard *k)
{
    k->phase=IDLE;k->started=0u;
}
int cs_mac_keyboard_step(cs_mac_keyboard *k,const cs_mac_io *io,
                         uint32_t tick,cs_m0110_event *out)
{
    int decoded;
    if(k->phase==FAILED)return CS_MAC_KEY_TIMEOUT;
    if(k->phase==IDLE){
        /* Clear only the shift flag. Preserve all non-shift ACR bits. */
        (void)io->read(io->context,SHIFT);
        mode(io,0x1cu);
        io->write(io->context,SHIFT,0x14u);
        k->started=tick;k->phase=OUT;
        return CS_MAC_KEY_WAIT;
    }
    if((io->read(io->context,IFR)&0x04u)==0u){
        if((uint32_t)(tick-k->started)>=8u){
            mode(io,0u);k->phase=FAILED;return CS_MAC_KEY_TIMEOUT;
        }
        return CS_MAC_KEY_WAIT;
    }
    if(k->phase==OUT){
        (void)io->read(io->context,SHIFT);
        mode(io,0x0cu);k->started=tick;k->phase=IN;
        return CS_MAC_KEY_WAIT;
    }
    decoded=cs_m0110_reply(io->read(io->context,SHIFT),out);
    k->phase=IDLE;
    if(decoded==CS_M0110_EVENT)return CS_MAC_KEY_EVENT;
    if(decoded==CS_M0110_EMPTY)return CS_MAC_KEY_EMPTY;
    return CS_MAC_KEY_INVALID;
}
