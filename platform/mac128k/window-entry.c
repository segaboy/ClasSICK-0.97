/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "rom.h"
#include "window-session.h"
#include "motion.h"
#include "../../arch/m68k/exclusion.h"
#include "../../core/services/traps.h"

/* SPEC-0072 opt-in native window entry. The SCC MMIO profile below is an
 * ENGINEERING HYPOTHESIS, not a published exact-port claim: lowest aliases of
 * the published 128K SCC read (even) and write (odd) windows. A/B select on
 * address bit 1 and D/C on bit 2 are inferred and UNVERIFIED. Only control
 * ports are used. The motion core readback gate (both marker writes before
 * both reads) stays the alias detector; failure is a terminal fault. */
#define CS_MAC_SCC_READ_B  UINT32_C(0x900000)
#define CS_MAC_SCC_READ_A  UINT32_C(0x900002)
#define CS_MAC_SCC_WRITE_B UINT32_C(0xB00001)
#define CS_MAC_SCC_WRITE_A UINT32_C(0xB00003)
#define CS_MAC_VIA         UINT32_C(0xEFE1FE)
#ifndef CLASSICK_MAC_DCD_INVERT
#define CLASSICK_MAC_DCD_INVERT 0 /* raw RR0 DCD sense UNVERIFIED */
#endif
#if CLASSICK_MAC_DCD_INVERT < 0 || CLASSICK_MAC_DCD_INVERT > 3
#error "CLASSICK_MAC_DCD_INVERT must be 0..3"
#endif

volatile cs_mac_diagnostics cs_mac_diag;
_Static_assert(offsetof(cs_mac_diagnostics,traps)==16,"Trap assembly offset");
_Static_assert(offsetof(cs_mac_diagnostics,fault)==24,"Fault assembly offset");
_Static_assert(offsetof(cs_trap_frame,a[7])==60 && offsetof(cs_trap_frame,pc)==64 &&
    offsetof(cs_trap_frame,sr)==68,"Native Line-A frame offsets");
extern void cs_mac_probe(void);
extern void cs_mac_fault(void);
/* window-irqs.S: all-register-saving IPL7 veneers calling the C entry below
 * with source 1, 2 or 3 respectively. */
extern void cs_mac_window_via_irq(void);
extern void cs_mac_window_scc_irq(void);
extern void cs_mac_window_combined_irq(void);
extern void cs_mac_window_install_vectors(void);
void cs_mac_window_interrupt(uint32_t source);

/* Own compact diagnostic record; not an ABI, never part of the 64B prefix. */
struct cs_mac_window_trace {
    uint32_t tag,version,bytes,source_irq,vbl,scc,combined,services;
    uint32_t baselines,losses,queued,peak,deliveries;
    int32_t pointer_x,pointer_y,window_x,window_y;
    uint32_t pattern,phase,motion_stage,raw_a,raw_b,buttons;
    int32_t motion_cause,cause;
};
volatile struct cs_mac_window_trace cs_mac_trace;

static cs_mac_window_session session __attribute__((section(".loaded_app")));
static struct cs_mac_motion motion;
static volatile uint32_t terminal;
_Static_assert(sizeof(struct cs_mac_motion)+sizeof(struct cs_mac_window_trace)<=320,
    "Window entry BSS budget");

__attribute__((noreturn)) static void halt(uint32_t code)
{
    cs_mac_diag.fault=code;
    for(;;) { }
}
void cs_mac_line_a(cs_trap_frame *f)
{
    /* Native window profile has no guest ABI; any Line-A is terminal. */
    (void)f;halt(7);
}

/* 16 NOPs = 64 clocks: >=8us at <=8MHz, covering SCC recovery plus the
 * 4-PCLK reset extra at 3.672MHz. Issued after every SCC byte access. */
static inline void scc_guard(void)
{
    __asm__ volatile("nop; nop; nop; nop; nop; nop; nop; nop; "
                     "nop; nop; nop; nop; nop; nop; nop; nop" ::: "memory");
}
static uint8_t via_read(void *context,unsigned reg)
{
    (void)context;
    return *(volatile uint8_t *)(uintptr_t)(CS_MAC_VIA+reg*512u);
}
static void via_write(void *context,unsigned reg,uint8_t value)
{
    (void)context;
    *(volatile uint8_t *)(uintptr_t)(CS_MAC_VIA+reg*512u)=value;
}
static volatile uint8_t *scc_ctrl_read(uint32_t channel)
{
    return (volatile uint8_t *)(uintptr_t)(channel==CS_MAC_SCC_A?CS_MAC_SCC_READ_A:CS_MAC_SCC_READ_B);
}
static volatile uint8_t *scc_ctrl_write(uint32_t channel)
{
    return (volatile uint8_t *)(uintptr_t)(channel==CS_MAC_SCC_A?CS_MAC_SCC_WRITE_A:CS_MAC_SCC_WRITE_B);
}
static uint8_t scc_read(void *cookie,uint32_t channel,uint32_t reg)
{
    uint8_t v;
    (void)cookie;reg&=15u;
    if(reg){*scc_ctrl_write(channel)=(uint8_t)reg;scc_guard();}
    v=*scc_ctrl_read(channel);scc_guard();
    return v;
}
static void scc_write(void *cookie,uint32_t channel,uint32_t reg,uint8_t value)
{
    (void)cookie;reg&=15u;
    if(reg){*scc_ctrl_write(channel)=(uint8_t)reg;scc_guard();}
    *scc_ctrl_write(channel)=value;scc_guard();
}
static uint8_t motion_pb(void *cookie)
{
    return via_read(cookie,0);
}
static void saturate(volatile uint32_t *v)
{
    if(*v!=UINT32_MAX)*v=*v+1u;
}

/* ISR context only (IPL7 veneer): controller stop is the sole cleanup. */
static void isr_fatal(uint32_t code,int32_t cause)
{
    via_write(0,14,0x7F);
    (void)cs_mac_motion_stop(&motion);
    cs_mac_trace.cause=cause;cs_mac_diag.fault=code;terminal=1u;
}
void cs_mac_window_interrupt(uint32_t source)
{
    int vbl=0,r;
    if(terminal)return;
    if(source<1u || source>3u){isr_fatal(14,(int32_t)source);return;}
    cs_mac_trace.source_irq=source;
    if(source==2u)saturate(&cs_mac_trace.scc);
    if(source==3u)saturate(&cs_mac_trace.combined);
    if(via_read(0,13)&0x02u){
        (void)via_read(0,1); /* CA1 acknowledge only; PB not sampled here */
        vbl=1;saturate(&cs_mac_diag.ticks);saturate(&cs_mac_trace.vbl);
    }
    if(source==1u && !vbl)return; /* empty VIA: no fabricated baseline */
    r=cs_mac_motion_service(&motion); /* one finite pass; RR3 authoritative */
    if(r==CS_MAC_MOTION_ARGUMENT || r==CS_MAC_MOTION_STATE || r==CS_MAC_MOTION_FAULT)
        isr_fatal(15,r);
}

/* Foreground: quiesce under exclusion, stop the coordinator outside it once. */
__attribute__((noreturn)) static void fg_fatal(uint32_t code,int32_t cause)
{
    uint32_t token=cs_m68k_foreground_enter(0);
    if(!terminal){
        via_write(0,14,0x7F);(void)cs_mac_motion_stop(&motion);
        cs_mac_trace.cause=cause;cs_mac_diag.fault=code;terminal=1u;
    }
    cs_m68k_foreground_leave(0,token);
    (void)cs_foreground_stop(&session.foreground);
    for(;;) { }
}
static uint32_t observe(void)
{
    uint32_t token=cs_m68k_foreground_enter(0),dead=terminal;
    if(!dead){
        cs_mac_trace.services=motion.services;cs_mac_trace.baselines=motion.baselines;
        cs_mac_trace.losses=motion.losses;cs_mac_trace.queued=session.inbox.count;
        cs_mac_trace.peak=motion.peak_queued;cs_mac_trace.motion_stage=motion.stage;
        cs_mac_trace.motion_cause=motion.cause;cs_mac_trace.raw_a=motion.last_raw_a;
        cs_mac_trace.raw_b=motion.last_raw_b;cs_mac_trace.buttons=motion.last_buttons;
        cs_mac_trace.pointer_x=session.input.pointer.x;cs_mac_trace.pointer_y=session.input.pointer.y;
        cs_mac_trace.window_x=session.input.window.x;cs_mac_trace.window_y=session.input.window.y;
        cs_mac_trace.pattern=session.input.window.pattern;cs_mac_trace.phase=session.foreground.phase;
    }
    cs_m68k_foreground_leave(0,token);
    return dead;
}

void cs_mac_main(void)
{
    cs_foreground_hooks hooks;cs_mac_io io;cs_mac_motion_io mio;int r;
    io.context=0;io.read=via_read;io.write=via_write;
    terminal=0u;
    cs_mac_diag.magic=UINT32_C(0x43533039);cs_mac_diag.stage=1;
    cs_mac_trace.tag=UINT32_C(0x4D575031);cs_mac_trace.version=1u;
    cs_mac_trace.bytes=(uint32_t)sizeof(struct cs_mac_window_trace);
    cs_mac_diag.image_crc=cs_mac_image_crc((const volatile uint8_t *)(uintptr_t)0x400000,CS_MAC_ROM_BYTES);
    if(cs_mac_diag.image_crc!=*(const volatile uint32_t *)(uintptr_t)0x400100)halt(1);
    cs_mac_diag.stage=2;
    cs_mac_probe();if(cs_mac_diag.traps!=1)halt(2);
    cs_mac_diag.stage=3;
    cs_mac_via_init(&io);
    cs_mac_diag.stage=4;
    hooks.enter=cs_m68k_foreground_enter;hooks.leave=cs_m68k_foreground_leave;hooks.cookie=0;
    /* begin initializes the NOLOAD session; nothing old is read. */
    r=cs_mac_window_begin(&session,(uint8_t *)(uintptr_t)0x1A700,CS_MAC_SCREEN_BYTES,&hooks);
    if(r!=CS_MAC_WINDOW_OK){cs_mac_trace.cause=r;halt(3);}
    cs_mac_diag.stage=5;
    mio.cookie=0;mio.read=scc_read;mio.write=scc_write;mio.pb=motion_pb;
    r=cs_mac_motion_prepare(&motion,&mio,&session.inbox,(uint32_t)CLASSICK_MAC_DCD_INVERT);
    if(r!=CS_MAC_MOTION_OK){cs_mac_trace.cause=r;halt(12);}
    cs_mac_diag.stage=6;
    /* Still IPL7 and quiescent: install level 1/2/3 autovectors. */
    cs_mac_window_install_vectors();
    r=cs_mac_motion_arm(&motion);
    if(r!=CS_MAC_MOTION_OK){cs_mac_trace.cause=r;halt(13);}
    via_write(0,14,0x82); /* CA1 enable last */
    cs_mac_diag.stage=16u;cs_mac_diag.fault=0u;
    __asm__ volatile("andi.w #0xF8FF,%%sr" ::: "cc","memory");
    for(;;){
        if(observe())fg_fatal(17,0);
        r=cs_foreground_step(&session.foreground);
        if(r!=CS_FG_OK && r!=CS_FG_EMPTY && r!=CS_FG_BASELINED &&
           r!=CS_FG_DELIVERED && r!=CS_FG_RESET)fg_fatal(16,r);
        if(r==CS_FG_DELIVERED)saturate(&cs_mac_trace.deliveries);
        cs_mac_diag.presses=session.input.counter.presses;
    }
}
