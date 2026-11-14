/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "keyboard-pointer.h"

int cs_pc286_keyboard_pointer_begin(cs_pc286_keyboard_pointer *state)
{
    if(!state)return 0;
    state->held=0;
    return 1;
}

int cs_pc286_keyboard_pointer_key(cs_pc286_keyboard_pointer *state,uint16_t key,
                                 cs_pointer_sample *out)
{
    cs_pointer_sample next;
    unsigned scan;
    if(!state || !out || state->held>1u)return 0;
    next.dx=0;next.dy=0;next.buttons=state->held;
    scan=(unsigned)(key>>8);
    if(scan==0x4bu)next.dx=-16;
    else if(scan==0x4du)next.dx=16;
    else if(scan==0x48u)next.dy=-16;
    else if(scan==0x50u)next.dy=16;
    else if((key&0xffu)==0x20u)next.buttons^=1u;
    else return 0;
    state->held=(unsigned)next.buttons;*out=next;
    return 1;
}
