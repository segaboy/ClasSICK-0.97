/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/uefi/keyboard.h"
#include "acpi-fixture.h"
#include "ps2-bus-fixture.h"
#include <stdio.h>
#include <stdlib.h>
enum{WIDTH=640,HEIGHT=480,PITCH=648,BYTES=PITCH*4*HEIGHT,GUARD=32};
static unsigned char fb[BYTES+2*GUARD],saved[BYTES],trace[144],map[48];
static bus_fixture controller;
static uint32_t timer;
static unsigned failures,mouse_frames;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"native pointer line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static uint32_t u32(const unsigned char *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static uint32_t tick(void *context,uint16_t port)
{(void)context;CHECK(port==0x608);timer=(timer+1000u)&0xFFFFFFu;return timer;}
static void observed(void *context,uint64_t ticks,const unsigned char *s,uint32_t notice)
{
    (void)context;(void)ticks;
    if(notice!=2)return;
    uint32_t count=u32(s+36),step=u32(s+76);
    if(count==0)memcpy(saved,fb+GUARD,BYTES);
    else if(count<=2 && step==0){
        ++mouse_frames;
        /* Independent512x342 cell bound, centered at(64,69), including padding. */
        for(size_t y=0;y<HEIGHT;++y)for(size_t byte=0;byte<PITCH*4u;++byte)
            if(!(y>=355 && y<375 && byte>=416u*4u && byte<496u*4u))
                CHECK(saved[y*PITCH*4u+byte]==fb[GUARD+y*PITCH*4u+byte]);
    }
}
static int untouched(void)
{for(size_t i=0;i<sizeof fb;++i)if(fb[i]!=0xA5)return 0;return 1;}
int main(void)
{
    void *arena=malloc(CS_LOADER_ARENA_BYTES);if(!arena)return 2;
    bf_init(&controller);timer=0;memset(fb,0xA5,sizeof fb);memset(trace,0xE7,sizeof trace);
    memset(map,0,sizeof map);fx_put(map,9,4);fx_put(map+8,FX_PHYS,8);fx_put(map+16,FX_PHYS,8);fx_put(map+24,16,8);
    fx_build(0,276);fx[FX_FADT+8]=6;fx[FX_FADT+109]=2;fx_seal(fx+FX_FADT);
    cs_native_memory memory={map,sizeof map,48,1,FX_PHYS,FX_SIZE,(uintptr_t)fx};
    cs_native_devices devices={&memory,tick,NULL,fb+GUARD,arena};cs_ps2_io io=bf_io(&controller);
    cs_uefi_handoff h;memset(&h,0,sizeof h);h.stage=CS_UEFI_EXITED;h.rsdp=FX_PHYS+FX_RSDP;h.arena_size=CS_LOADER_ARENA_BYTES;
    h.framebuffer.base=UINT64_C(0x80000000);h.framebuffer.size=BYTES;h.framebuffer.width=WIDTH;h.framebuffer.height=HEIGHT;h.framebuffer.pitch=PITCH;h.framebuffer.format=1;
    CHECK(cs_native_keyboard_observed(&h,1,&devices,&io,60,NULL,NULL,NULL,NULL)==CS_KBD_ARGUMENT);
    h.stage=CS_UEFI_FAILED;CHECK(cs_native_keyboard_observed(&h,1,&devices,&io,60,trace,NULL,NULL,NULL)==CS_KBD_NOT_EXITED);
    h.stage=CS_UEFI_EXITED;CHECK(cs_native_keyboard_observed(&h,0,&devices,&io,60,trace,NULL,NULL,NULL)==CS_KBD_NOT_READY);
    fx[FX_FADT+109]=0;fx_seal(fx+FX_FADT);
    CHECK(cs_native_keyboard_observed(&h,1,&devices,&io,60,trace,NULL,NULL,NULL)==CS_KBD_NO_8042);
    CHECK(controller.key.writes==0 && controller.mouse_writes==0 && timer==0 && untouched());
    fx[FX_FADT+109]=2;fx_seal(fx+FX_FADT);controller.bad_test=1;
    CHECK(cs_native_keyboard_observed(&h,1,&devices,&io,60,trace,NULL,NULL,NULL)==CS_KBD_KEYBOARD);
    CHECK(u32(trace+4)==2 && u32(trace+12)==CS_PS2_CONTROLLER && u32(trace+32)==0 && untouched());
    bf_init(&controller);timer=0;memset(trace,0xE7,sizeof trace);
    static const uint8_t bytes[]={9,0,0,9,0,0,8,0,0,9,0,0,0x29,0xF0,0x29,0x76};
    memcpy(controller.live_bytes,bytes,sizeof bytes);controller.live_count=sizeof bytes;
    for(unsigned i=0;i<12;++i)controller.live_flags[i]=0x20;
    CHECK(cs_native_keyboard_observed(&h,1,&devices,&io,60,trace,NULL,observed,NULL)==CS_KBD_ESCAPED);
    CHECK(u32(trace)==CS_KBD_TRACE_MAGIC && u32(trace+4)==2 && u32(trace+36)==3 && u32(trace+40)==1 && mouse_frames==2);
    CHECK(controller.key.violations==0 && controller.ready);
    for(unsigned i=112;i<sizeof trace;++i)CHECK(trace[i]==0xE7);
    for(unsigned i=0;i<GUARD;++i)CHECK(fb[i]==0xA5 && fb[GUARD+BYTES+i]==0xA5);
    for(size_t y=0;y<HEIGHT;++y)for(size_t x=WIDTH*4u;x<PITCH*4u;++x)CHECK(fb[GUARD+y*PITCH*4u+x]==0xA5);
    free(arena);puts(failures?"native pointer: FAIL":"native pointer gates/mouse+Space/scene/trace: PASS");return failures!=0;
}
