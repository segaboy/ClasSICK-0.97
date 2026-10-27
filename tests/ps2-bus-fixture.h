/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PS2_BUS_FIXTURE_H
#define CLASSICK_PS2_BUS_FIXTURE_H
#include "ps2-fixture.h"
/* Original AUX extension of the qualified authored keyboard transcript. */
typedef struct {
    ps2_fixture key;cs_ps2_io legacy;
    unsigned mouse,prefix,ready,live_at,live_count,mouse_writes;
    uint8_t config,bad_test,live_bytes[64],live_flags[64];
} bus_fixture;
static void bf_init(bus_fixture *f)
{
    memset(f,0,sizeof *f);pf_init(&f->key);f->legacy=pf_io(&f->key);
}
static uint8_t bf_read(void *p,uint16_t port)
{
    bus_fixture *f=p;
    if(port==0x64 && f->ready && f->key.head==f->key.tail && f->live_at<f->live_count){
        unsigned at=f->live_at++;pf_byte(&f->key,f->live_bytes[at],f->live_flags[at]);
    }
    return f->legacy.read(f->legacy.context,port);
}
static void bf_write(void *p,uint16_t port,uint8_t byte)
{
    bus_fixture *f=p;
    if(!f->mouse){
        if(f->key.live && port==0x64 && byte==0xAD){f->mouse=1;f->config=(uint8_t)(f->key.configuration|0x10u);}
        else{f->legacy.write(f->legacy.context,port,byte);return;}
    }
    ++f->mouse_writes;
    if(f->ready || (port!=0x60 && port!=0x64)){++f->key.violations;return;}
    if(port==0x64){
        if(f->prefix){++f->key.violations;return;}
        if(byte==0xAD)f->config|=0x10;
        else if(byte==0xA7)f->config|=0x20;
        else if(byte==0xA9)pf_byte(&f->key,f->bad_test,0);
        else if(byte==0xA8)f->config&=(uint8_t)~0x20u;
        else if(byte==0xD4)f->prefix=1;
        else if(byte==0x20)pf_byte(&f->key,f->config,0);
        else if(byte==0xAE)f->config&=(uint8_t)~0x10u;
        else ++f->key.violations;
    }else{
        if(!f->prefix){++f->key.violations;return;}f->prefix=0;
        if(byte!=0xF5 && byte!=0xFF && byte!=0xF6 && byte!=0xF4){++f->key.violations;return;}
        pf_byte(&f->key,0xFA,0x20);
        if(byte==0xFF){pf_byte(&f->key,0xAA,0x20);pf_byte(&f->key,0,0x20);}
        if(byte==0xF4)f->ready=1; /* streaming starts at F4 ACK */
    }
}
static cs_ps2_io bf_io(bus_fixture *f)
{
    cs_ps2_io io={bf_read,bf_write,f};return io;
}
#endif
