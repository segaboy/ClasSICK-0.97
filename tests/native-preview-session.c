/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* SPEC-0050/TEST-0059: keyboard.c compiled with CLASSICK_PC_PREVIEW_SESSION=1,
   CLASSICK_PC_MOUSE=1, CLASSICK_OUTLINE_DRAWING=1, CLASSICK_SELECTED_DRAWING=1. */
#include "../platform/uefi/keyboard.h"
#include "acpi-fixture.h"
#include "ps2-bus-fixture.h"
#include <stdio.h>
#include <stdlib.h>
enum{WIDTH=640,HEIGHT=480,PITCH=648,BYTES=PITCH*4*HEIGHT,GUARD=32};
#define HZ UINT64_C(3579545)
/* Input arrives after 62 timer-seconds (loop elapsed >=61); the documented
   end is a raw value wider than the 24-bit counter after 67 seconds. */
#define INJECT (UINT64_C(62)*HZ)
#define FAULT (UINT64_C(67)*HZ)
static unsigned char fb[BYTES+2*GUARD],saved[BYTES],trace[144],map[48];
static bus_fixture controller;
static uint32_t timer,stride,frames61,late_frames;
static uint64_t elapsed;
static int armed,seen61;
static unsigned failures;
/* Mouse press, held, release; Space; Escape; Space after Escape. */
static const uint8_t script[]={9,0,0,9,0,0,8,0,0,0x29,0xF0,0x29,0x76,0xF0,0x76,0x29,0xF0,0x29};
#define CHECK(x) do{if(!(x)){fprintf(stderr,"native preview line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static uint32_t u32(const unsigned char *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static uint32_t tick(void *context,uint16_t port)
{
    (void)context;CHECK(port==0x608);
    if(elapsed>=FAULT)return UINT32_MAX;
    if(elapsed>=INJECT)controller.live_count=(unsigned)sizeof script;
    timer=(timer+stride)&0xFFFFFFu;elapsed+=stride;return timer;
}
static void observed(void *context,uint64_t ticks,const unsigned char *s,uint32_t notice)
{
    (void)context;(void)ticks;
    /* Fine 1000-tick steps through controller startup, then ~28 ms turns. */
    if(notice==2 && !armed && u32(s+32)==1){armed=1;stride=100000;}
    if(notice==0 && !seen61 && elapsed>=INJECT){
        seen61=1;frames61=u32(s+32);
        CHECK(u32(s+8)==CS_KBD_RUNNING && u32(s+24)>=61 && u32(s+36)==0 && u32(s+76)==0);
        memcpy(saved,fb+GUARD,BYTES);
    }
    if(notice==2 && seen61){
        size_t changed=0;
        ++late_frames;
        for(size_t y=0;y<HEIGHT;++y)for(size_t byte=0;byte<PITCH*4u;++byte){
            size_t i=y*PITCH*4u+byte;
            if(y>=355u && y<375u && byte>=416u*4u && byte<496u*4u){
                if(saved[i]!=fb[GUARD+i])++changed;
            }else CHECK(saved[i]==fb[GUARD+i]);
        }
        CHECK(changed>0);
    }
}
static int untouched(void)
{for(size_t i=0;i<sizeof fb;++i)if(fb[i]!=0xA5)return 0;return 1;}
static uint32_t run(const cs_uefi_handoff *h,const cs_native_devices *dev,uint32_t seconds)
{
    cs_ps2_io io;
    bf_init(&controller);memcpy(controller.live_bytes,script,sizeof script);
    for(unsigned i=0;i<9;++i)controller.live_flags[i]=0x20;
    timer=0;stride=1000;elapsed=0;armed=0;seen61=0;frames61=0;late_frames=0;
    memset(fb,0xA5,sizeof fb);memset(trace,0xE7,sizeof trace);io=bf_io(&controller);
    return cs_native_keyboard_observed(h,1,dev,&io,seconds,trace,NULL,observed,NULL);
}
static void verify(uint32_t result,uint32_t seconds)
{
    CHECK(result==CS_KBD_VALUE);
    /* Static scene: one frame from startup until the first input after 60 s. */
    CHECK(armed && seen61 && frames61==1);
    CHECK(u32(trace)==CS_KBD_TRACE_MAGIC && u32(trace+4)==2 && u32(trace+8)==CS_KBD_VALUE);
    CHECK(u32(trace+20)==seconds && u32(trace+24)>=65);
    /* Mouse edge counted once despite held packet; Space after Escape accepted. */
    CHECK(u32(trace+36)==3 && u32(trace+40)==1 && u32(trace+76)==2);
    CHECK(late_frames>=1 && u32(trace+32)==1u+late_frames);
    CHECK(controller.live_at==sizeof script && controller.key.violations==0 && controller.ready);
    for(unsigned i=112;i<sizeof trace;++i)CHECK(trace[i]==0xE7);
    for(unsigned i=0;i<GUARD;++i)CHECK(fb[i]==0xA5 && fb[GUARD+BYTES+i]==0xA5);
    for(size_t y=0;y<HEIGHT;++y)for(size_t x=WIDTH*4u;x<PITCH*4u;++x)CHECK(fb[GUARD+y*PITCH*4u+x]==0xA5);
}
int main(void)
{
    void *arena=malloc(CS_LOADER_ARENA_BYTES);if(!arena)return 2;
    bf_init(&controller);timer=0;stride=1000;elapsed=0;
    memset(fb,0xA5,sizeof fb);memset(trace,0xE7,sizeof trace);
    memset(map,0,sizeof map);fx_put(map,9,4);fx_put(map+8,FX_PHYS,8);fx_put(map+16,FX_PHYS,8);fx_put(map+24,16,8);
    fx_build(0,276);fx[FX_FADT+8]=6;fx[FX_FADT+109]=2;fx_seal(fx+FX_FADT);
    cs_native_memory memory={map,sizeof map,48,1,FX_PHYS,FX_SIZE,(uintptr_t)fx};
    cs_native_devices devices={&memory,tick,NULL,fb+GUARD,arena};cs_ps2_io io=bf_io(&controller);
    cs_uefi_handoff h;memset(&h,0,sizeof h);h.stage=CS_UEFI_EXITED;h.rsdp=FX_PHYS+FX_RSDP;h.arena_size=CS_LOADER_ARENA_BYTES;
    h.framebuffer.base=UINT64_C(0x80000000);h.framebuffer.size=BYTES;h.framebuffer.width=WIDTH;h.framebuffer.height=HEIGHT;h.framebuffer.pitch=PITCH;h.framebuffer.format=1;
    CHECK(cs_native_keyboard_observed(&h,1,&devices,&io,60,NULL,NULL,NULL,NULL)==CS_KBD_ARGUMENT);
    CHECK(cs_native_keyboard_observed(&h,1,&devices,&io,0,trace,NULL,NULL,NULL)==CS_KBD_ARGUMENT && u32(trace+8)==CS_KBD_ARGUMENT);
    CHECK(cs_native_keyboard_observed(&h,1,&devices,&io,3601,trace,NULL,NULL,NULL)==CS_KBD_ARGUMENT);
    h.stage=CS_UEFI_FAILED;CHECK(cs_native_keyboard_observed(&h,1,&devices,&io,60,trace,NULL,NULL,NULL)==CS_KBD_NOT_EXITED);
    h.stage=CS_UEFI_EXITED;CHECK(cs_native_keyboard_observed(&h,0,&devices,&io,60,trace,NULL,NULL,NULL)==CS_KBD_NOT_READY);
    fx[FX_FADT+109]=0;fx_seal(fx+FX_FADT);
    CHECK(cs_native_keyboard_observed(&h,1,&devices,&io,60,trace,NULL,NULL,NULL)==CS_KBD_NO_8042);
    CHECK(controller.key.writes==0 && controller.mouse_writes==0 && timer==0 && untouched());
    fx[FX_FADT+109]=2;fx_seal(fx+FX_FADT);controller.bad_test=1;
    CHECK(cs_native_keyboard_observed(&h,1,&devices,&io,60,trace,NULL,NULL,NULL)==CS_KBD_KEYBOARD);
    CHECK(u32(trace+12)==CS_PS2_CONTROLLER && u32(trace+32)==0 && untouched());
    verify(run(&h,&devices,60),60);
    verify(run(&h,&devices,1),1);
    free(arena);
    puts(failures?"native preview session: FAIL":"native preview session past60/Escape/input/fail-closed: PASS");
    return failures!=0;
}
