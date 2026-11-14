/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc286/keyboard-pointer.h"
#include <stdio.h>

int main(void)
{
    unsigned held;uint32_t key;unsigned long checks=0;
    cs_pc286_keyboard_pointer state;cs_pointer_sample sample;
    if(cs_pc286_keyboard_pointer_begin(NULL) || !cs_pc286_keyboard_pointer_begin(&state) || state.held)return 1;
    for(held=0;held<2u;++held)for(key=0;key<65536u;++key) {
        unsigned scan=(unsigned)(key>>8);int known=1;int32_t dx=0,dy=0;unsigned next=held;
        if(scan==0x4bu)dx=-16;
        else if(scan==0x4du)dx=16;
        else if(scan==0x48u)dy=-16;
        else if(scan==0x50u)dy=16;
        else if((key&255u)==32u)next=1u-held;
        else known=0;
        state.held=held;sample.dx=12345;sample.dy=-54321;sample.buttons=0xabcdef01u;
        if(cs_pc286_keyboard_pointer_key(&state,(uint16_t)key,&sample)!=known || state.held!=next)return 2;
        if(known) {
            if(sample.dx!=dx || sample.dy!=dy || sample.buttons!=next)return 3;
        } else if(sample.dx!=12345 || sample.dy!=-54321 || sample.buttons!=0xabcdef01u)return 4;
        ++checks;
    }
    state.held=2;sample.dx=12345;sample.dy=-54321;sample.buttons=0xabcdef01u;
    if(cs_pc286_keyboard_pointer_key(&state,0x3920,&sample) || state.held!=2 ||
       sample.dx!=12345 || sample.dy!=-54321 || sample.buttons!=0xabcdef01u)return 5;
    state.held=0;
    if(cs_pc286_keyboard_pointer_key(NULL,0x3920,&sample) ||
       cs_pc286_keyboard_pointer_key(&state,0x3920,NULL) || state.held)return 6;
    printf("PASS %lu key/state combinations and refusal checks\n",checks);return 0;
}
