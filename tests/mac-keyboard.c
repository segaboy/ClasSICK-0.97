/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/mac128k/keyboard.h"
#include <assert.h>
#include <stdio.h>
typedef struct { uint8_t regs[16]; unsigned reads,writes; } fake;
static uint8_t rd(void *v,unsigned r)
{
    fake *f=v;uint8_t x=f->regs[r];f->reads++;
    if(r==10u)f->regs[13]&=(uint8_t)~4u;
    return x;
}
static void wr(void *v,unsigned r,uint8_t x)
{
    fake *f=v;f->writes++;f->regs[r]=x;
}
static void cycle(cs_mac_keyboard *k,cs_mac_io *io,fake *f,uint8_t reply,
                  int expected,uint32_t pos,uint32_t pressed)
{
    cs_m0110_event e={999u,999u};
    assert(cs_mac_keyboard_step(k,io,1u,&e)==CS_MAC_KEY_WAIT);
    assert(f->regs[10]==0x14u && f->regs[11]==0xffu);
    assert(cs_mac_keyboard_step(k,io,1u,&e)==CS_MAC_KEY_WAIT);
    f->regs[13]|=4u;
    assert(cs_mac_keyboard_step(k,io,2u,&e)==CS_MAC_KEY_WAIT);
    assert(f->regs[11]==0xefu);
    f->regs[10]=reply;f->regs[13]|=4u;
    assert(cs_mac_keyboard_step(k,io,2u,&e)==expected);
    assert(e.position==pos && e.pressed==pressed);
}
int main(void)
{
    fake f={{0},0,0};cs_mac_io io={&f,rd,wr};cs_mac_keyboard k;
    cs_m0110_event e={99u,88u};
    f.regs[11]=0xe3u;cs_mac_keyboard_begin(&k);
    cycle(&k,&io,&f,0x01u,CS_MAC_KEY_EVENT,0u,1u);
    cycle(&k,&io,&f,0x81u,CS_MAC_KEY_EVENT,0u,0u);
    cycle(&k,&io,&f,0x03u,CS_MAC_KEY_EVENT,1u,1u);
    cycle(&k,&io,&f,0x83u,CS_MAC_KEY_EVENT,1u,0u);
    cycle(&k,&io,&f,0x7bu,CS_MAC_KEY_EMPTY,999u,999u);
    cycle(&k,&io,&f,0x00u,CS_MAC_KEY_INVALID,999u,999u);
    assert(cs_mac_keyboard_step(&k,&io,10u,&e)==CS_MAC_KEY_WAIT);
    assert(cs_mac_keyboard_step(&k,&io,17u,&e)==CS_MAC_KEY_WAIT);
    assert(cs_mac_keyboard_step(&k,&io,18u,&e)==CS_MAC_KEY_TIMEOUT);
    assert(f.regs[11]==0xe3u);
    { unsigned before=f.reads+f.writes;
      assert(cs_mac_keyboard_step(&k,&io,19u,&e)==CS_MAC_KEY_TIMEOUT);
      assert(before==f.reads+f.writes); }
    assert(e.position==99u && e.pressed==88u);
    cs_mac_keyboard_begin(&k);
    assert(cs_mac_keyboard_step(&k,&io,UINT32_MAX-2u,&e)==CS_MAC_KEY_WAIT);
    f.regs[13]|=4u;
    assert(cs_mac_keyboard_step(&k,&io,UINT32_MAX-1u,&e)==CS_MAC_KEY_WAIT);
    assert(cs_mac_keyboard_step(&k,&io,5u,&e)==CS_MAC_KEY_WAIT);
    assert(cs_mac_keyboard_step(&k,&io,6u,&e)==CS_MAC_KEY_TIMEOUT);
    assert(f.regs[11]==0xe3u);
    puts("mac keyboard transport checks passed");return 0;
}
