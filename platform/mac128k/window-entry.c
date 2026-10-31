/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "rom.h"
#include "window-session.h"
#include "motion.h"
#ifndef CLASSICK_MAC_TIMER_POLL_PROBE
#define CLASSICK_MAC_TIMER_POLL_PROBE 0
#endif
#if CLASSICK_MAC_TIMER_POLL_PROBE != 0 && CLASSICK_MAC_TIMER_POLL_PROBE != 1
#error CLASSICK_MAC_TIMER_POLL_PROBE must be 0 or 1
#endif
#if CLASSICK_MAC_TIMER_POLL_PROBE
#include "timer-poll.h"
#endif
#ifndef CLASSICK_MAC_TIMER_RECOVERY_PROBE
#define CLASSICK_MAC_TIMER_RECOVERY_PROBE 0
#endif
#if CLASSICK_MAC_TIMER_RECOVERY_PROBE != 0 && CLASSICK_MAC_TIMER_RECOVERY_PROBE != 1
#error CLASSICK_MAC_TIMER_RECOVERY_PROBE must be0 or1
#endif
#ifndef CLASSICK_MAC_TIMER_DIVISOR
#define CLASSICK_MAC_TIMER_DIVISOR 1
#endif
#if CLASSICK_MAC_TIMER_DIVISOR != 1 && CLASSICK_MAC_TIMER_DIVISOR != 4 && CLASSICK_MAC_TIMER_DIVISOR != 16
#error CLASSICK_MAC_TIMER_DIVISOR must be1,4 or16
#endif
#ifndef CLASSICK_MAC_TIMER_LIVE_PAIR
#define CLASSICK_MAC_TIMER_LIVE_PAIR 0
#endif
#if CLASSICK_MAC_TIMER_LIVE_PAIR != 0 && CLASSICK_MAC_TIMER_LIVE_PAIR != 1
#error CLASSICK_MAC_TIMER_LIVE_PAIR must be 0 or 1
#endif
#if CLASSICK_MAC_TIMER_DIVISOR != 1 && !CLASSICK_MAC_TIMER_LIVE_PAIR && !(CLASSICK_MAC_TIMER_RECOVERY_PROBE && CLASSICK_MAC_TIMER_REFRESH_PROBE)
#error Nondefault timer divisor requires live pair or recovery and refresh probes
#endif
#if CLASSICK_MAC_TIMER_RECOVERY_PROBE
#include "activity-window.h"
static cs_mac_activity_window cs_mac_activity_gate;
#endif
#include "input-probe.h"
#include "../../arch/m68k/exclusion.h"
#include "../../core/services/traps.h"

/* SPEC-0072 opt-in native window entry. The SCC MMIO profile below is an
 * ENGINEERING HYPOTHESIS, not a published exact-port claim: upper aliases of
 * the published 128K SCC read (even) and write (odd) windows. A/B select on
 * address bit 1 and D/C on bit 2 are inferred and UNVERIFIED. Only control
 * ports are used. The motion core readback gate (both DCD-mask writes before
 * both reads) stays the alias detector; failure is a terminal fault. */
#define CS_MAC_SCC_READ_B  UINT32_C(0x9FFFF8)
#define CS_MAC_SCC_READ_A  UINT32_C(0x9FFFFA)
#define CS_MAC_SCC_WRITE_B UINT32_C(0xBFFFF9)
#define CS_MAC_SCC_WRITE_A UINT32_C(0xBFFFFB)
#define CS_MAC_VIA         UINT32_C(0xEFE1FE)
#ifndef CLASSICK_MAC_DCD_INVERT
#define CLASSICK_MAC_DCD_INVERT 0 /* raw RR0 DCD sense UNVERIFIED */
#endif
#if CLASSICK_MAC_DCD_INVERT < 0 || CLASSICK_MAC_DCD_INVERT > 3
#error "CLASSICK_MAC_DCD_INVERT must be 0..3"
#endif
#ifndef CLASSICK_MAC_IDLE_PROBE
#define CLASSICK_MAC_IDLE_PROBE 0
#endif
#if CLASSICK_MAC_IDLE_PROBE < 0 || CLASSICK_MAC_IDLE_PROBE > 1
#error "CLASSICK_MAC_IDLE_PROBE must be 0 or 1"
#endif
#if CLASSICK_MAC_TIMER_RECOVERY_PROBE && !(CLASSICK_MAC_TIMER_POLL_PROBE && CLASSICK_MAC_TIMER_ACTIVITY_PROBE && CLASSICK_MAC_IDLE_PROBE)
#error Recovery observation requires timer polling,activity and idle checkpoint
#endif
#if CLASSICK_MAC_TIMER_LIVE_PAIR && !(CLASSICK_MAC_TIMER_PAIR_PROBE && CLASSICK_MAC_TIMER_POLL_PROBE && CLASSICK_MAC_TIMER_REFRESH_PROBE)
#error Live pair requires pair callback,timer polling and refresh
#endif
#if CLASSICK_MAC_TIMER_LIVE_PAIR && (CLASSICK_MAC_IDLE_PROBE || CLASSICK_MAC_TIMER_ACTIVITY_PROBE || CLASSICK_MAC_TIMER_RECOVERY_PROBE || CLASSICK_MAC_FIRST_INPUT_PROBE || CLASSICK_MAC_PENDING_DEFER_PROBE || CLASSICK_MAC_PHASE_IDLE_PROBE)
#error Live pair forbids checkpoint diagnostics
#endif
#ifndef CLASSICK_MAC_TIMER_PHASE_WITNESS
#define CLASSICK_MAC_TIMER_PHASE_WITNESS 0
#endif
#if CLASSICK_MAC_TIMER_PHASE_WITNESS != 0 && CLASSICK_MAC_TIMER_PHASE_WITNESS != 1
#error CLASSICK_MAC_TIMER_PHASE_WITNESS must be 0 or 1
#endif
#if CLASSICK_MAC_TIMER_PHASE_WITNESS && !(CLASSICK_MAC_TIMER_LIVE_PAIR && CLASSICK_MAC_TIMER_DIVISOR == 4)
#error Phase witness requires live pair divisor 4
#endif
#if CLASSICK_MAC_TIMER_PAIR_PROBE && !CLASSICK_MAC_TIMER_LIVE_PAIR && !(CLASSICK_MAC_TIMER_POLL_PROBE && CLASSICK_MAC_TIMER_RECOVERY_PROBE && CLASSICK_MAC_TIMER_REFRESH_PROBE)
#error Pair probe requires timer polling,recovery and refresh profiles
#endif
#if CLASSICK_MAC_FIRST_INPUT_PROBE && !CLASSICK_MAC_IDLE_PROBE
#error "CLASSICK_MAC_FIRST_INPUT_PROBE requires CLASSICK_MAC_IDLE_PROBE=1"
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
#if CLASSICK_MAC_IDLE_PROBE
/* Diagnostic register mirror and self-branch, IPL7 held. Not a session stop. */
__attribute__((noreturn)) void cs_mac_window_idle_checkpoint(void);
#if CLASSICK_MAC_TIMER_ACTIVITY_PROBE
__attribute__((noreturn)) void cs_mac_activity_checkpoint(void);
#endif
#endif

/* Own compact diagnostic record; not an ABI, never part of the 64B prefix. */
struct cs_mac_window_trace {
    uint32_t tag,version,bytes,source_irq,vbl,scc,combined,services;
    uint32_t baselines,losses,queued,peak,deliveries;
    int32_t pointer_x,pointer_y,window_x,window_y;
    uint32_t pattern,phase,motion_stage,raw_a,raw_b,buttons;
    int32_t motion_cause,cause;
    uint32_t loss_reason,loss_rr3,loss_snap[3],loss_previous,loss_phase;
};
_Static_assert(sizeof(struct cs_mac_window_trace)==128,"MWP1 v2 size");
_Static_assert(offsetof(struct cs_mac_window_trace,loss_reason)==100,"MWP1 v1 prefix");
volatile struct cs_mac_window_trace cs_mac_trace;
#if CLASSICK_MAC_IDLE_PROBE
/* Offsets used by the register checkpoint in window-irqs.S. */
_Static_assert(offsetof(struct cs_mac_window_trace,services)==28,"MWR1 D1 offset");
_Static_assert(offsetof(struct cs_mac_window_trace,losses)==36,"MWR1 D2 offset");
_Static_assert(offsetof(struct cs_mac_window_trace,deliveries)==48,"MWR1 D3 offset");
_Static_assert(offsetof(struct cs_mac_window_trace,pointer_x)==52,"MWR1 A3 offset");
_Static_assert(offsetof(struct cs_mac_window_trace,pointer_y)==56,"MWR1 A4 offset");
_Static_assert(offsetof(struct cs_mac_window_trace,loss_reason)==100,"MWR1 D4 offset");
_Static_assert(offsetof(struct cs_mac_window_trace,loss_rr3)==104,"MWR1 D5 offset");
_Static_assert(offsetof(struct cs_mac_window_trace,loss_snap[0])==108,"MWR1 A0 offset");
_Static_assert(offsetof(struct cs_mac_window_trace,loss_snap[1])==112,"MWR1 A1 offset");
_Static_assert(offsetof(struct cs_mac_window_trace,loss_snap[2])==116,"MWR1 A2 offset");
_Static_assert(offsetof(struct cs_mac_window_trace,loss_previous)==120,"MWR1 D6 offset");
_Static_assert(offsetof(struct cs_mac_window_trace,loss_phase)==124,"MWR1 D7 offset");
#endif

static cs_mac_window_session session __attribute__((section(".loaded_app")));
#if CLASSICK_MAC_TIMER_POLL_PROBE
static cs_mac_timer_poll timer_motion;
#if CLASSICK_MAC_TIMER_ACTIVITY_PROBE
#if !CLASSICK_MAC_IDLE_PROBE
#error Activity checkpoint requires the opt-in idle checkpoint
#endif
/* Copied once under IPL7 immediately before the separate MAR1 checkpoint. */
volatile cs_mac_activity cs_mac_activity_trace;
_Static_assert(offsetof(cs_mac_activity,pairs)==12 &&
    offsetof(cs_mac_activity,pb_x_changes)==16 && offsetof(cs_mac_activity,pb_y_changes)==20 &&
    offsetof(cs_mac_activity,dcd_a_changes)==24 && offsetof(cs_mac_activity,dcd_b_changes)==28 &&
    offsetof(cs_mac_activity,button_changes)==32 && offsetof(cs_mac_activity,nonzero_queued)==40 &&
    offsetof(cs_mac_activity,buttononly_queued)==44 && offsetof(cs_mac_activity,losses)==48 &&
    offsetof(cs_mac_activity,first_previous)==60 && offsetof(cs_mac_activity,first_s0)==64 &&
    offsetof(cs_mac_activity,first_s1)==68 && offsetof(cs_mac_activity,first_reason)==72 &&
    offsetof(cs_mac_activity,first_service)==76,"MAR1 assembly offsets");
#endif
#define motion timer_motion.motion
#define native_motion_prepare(scc,via) cs_mac_timer_poll_prepare(&timer_motion,scc,via,&session.inbox,(uint32_t)CLASSICK_MAC_DCD_INVERT,UINT32_C(4096)/CLASSICK_MAC_TIMER_DIVISOR)
#define native_motion_arm() cs_mac_timer_poll_arm(&timer_motion)
#define native_motion_service() cs_mac_timer_poll_service(&timer_motion)
#define native_motion_stop() cs_mac_timer_poll_stop(&timer_motion)
_Static_assert(sizeof(timer_motion)+sizeof(struct cs_mac_window_trace)<=448,"Timer experiment BSS budget");
#else
#if CLASSICK_MAC_MOTION_TIMELINE
/* Named own-RAM symbol permits locating the diagnostic without a memory scan. */
struct cs_mac_motion cs_mac_timeline_motion;
volatile uint32_t cs_mac_timeline_address;
#define motion cs_mac_timeline_motion
#else
static struct cs_mac_motion motion;
#endif
#define native_motion_prepare(scc,via) cs_mac_motion_prepare(&motion,scc,&session.inbox,(uint32_t)CLASSICK_MAC_DCD_INVERT)
#define native_motion_arm() cs_mac_motion_arm(&motion)
#define native_motion_service() cs_mac_motion_service(&motion)
#define native_motion_stop() cs_mac_motion_stop(&motion)
#endif
static volatile uint32_t terminal;
_Static_assert(sizeof(struct cs_mac_motion)+sizeof(struct cs_mac_window_trace)<=
    (CLASSICK_MAC_MOTION_TIMELINE?544u:320u),
    "Window entry BSS budget");
#if CLASSICK_MAC_MOTION_TIMELINE && CLASSICK_MAC_TIMER_POLL_PROBE
#error Motion timeline requires the interrupt-based motion driver
#endif

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
#if CLASSICK_MAC_TIMER_PAIR_PROBE
#define PAIR_INL __attribute__((always_inline)) inline
#else
#define PAIR_INL
#endif
#if CLASSICK_MAC_TIMER_PAIR_PROBE
static PAIR_INL void scc_guard(void)
#else
static inline void scc_guard(void)
#endif
{
    __asm__ volatile("nop; nop; nop; nop; nop; nop; nop; nop; "
                     "nop; nop; nop; nop; nop; nop; nop; nop" ::: "memory");
}
static PAIR_INL uint8_t via_read(void *context,unsigned reg)
{
    (void)context;
    return *(volatile uint8_t *)(uintptr_t)(CS_MAC_VIA+reg*512u);
}
static void via_write(void *context,unsigned reg,uint8_t value)
{
    (void)context;
    *(volatile uint8_t *)(uintptr_t)(CS_MAC_VIA+reg*512u)=value;
}
static PAIR_INL volatile uint8_t *scc_ctrl_read(uint32_t channel)
{
    return (volatile uint8_t *)(uintptr_t)(channel==CS_MAC_SCC_A?CS_MAC_SCC_READ_A:CS_MAC_SCC_READ_B);
}
static PAIR_INL volatile uint8_t *scc_ctrl_write(uint32_t channel)
{
    return (volatile uint8_t *)(uintptr_t)(channel==CS_MAC_SCC_A?CS_MAC_SCC_WRITE_A:CS_MAC_SCC_WRITE_B);
}
static PAIR_INL uint8_t scc_read(void *cookie,uint32_t channel,uint32_t reg)
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
static PAIR_INL uint8_t motion_pb(void *cookie)
{
    return via_read(cookie,0);
}
#if CLASSICK_MAC_TIMER_PAIR_PROBE
/* Helpers are forced inline with constant args: no calls in this routine. */
void cs_mac_native_pair(void *cookie,uint8_t out[6])
{
    out[0]=motion_pb(cookie);
    out[1]=scc_read(cookie,CS_MAC_SCC_A,0);
    out[2]=scc_read(cookie,CS_MAC_SCC_B,0);
    out[3]=motion_pb(cookie);
    out[4]=scc_read(cookie,CS_MAC_SCC_A,0);
    out[5]=scc_read(cookie,CS_MAC_SCC_B,0);
}
#endif
static void saturate(volatile uint32_t *v)
{
    if(*v!=UINT32_MAX)*v=*v+1u;
}

/* ISR context only (IPL7 veneer): controller stop is the sole cleanup. */
static void isr_fatal(uint32_t code,int32_t cause)
{
    via_write(0,14,0x7F);
    (void)native_motion_stop();
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
#if !CLASSICK_MAC_TIMER_POLL_PROBE
    if(source==1u && !vbl)return; /* empty VIA: no fabricated baseline */
#else
    (void)vbl; /* T1 controller checks IFR6;empty IRQ never creates baseline. */
#endif
    r=native_motion_service(); /* one finite pass;profile chooses RR3 or T1. */
#if CLASSICK_MAC_MOTION_TIMELINE
    cs_mac_motion_timeline_commit(&motion,r);
#endif
    if(r==CS_MAC_MOTION_ARGUMENT || r==CS_MAC_MOTION_STATE || r==CS_MAC_MOTION_FAULT)
        isr_fatal(15,r);
}

/* Foreground: quiesce under exclusion, stop the coordinator outside it once. */
__attribute__((noreturn)) static void fg_fatal(uint32_t code,int32_t cause)
{
    uint32_t token=cs_m68k_foreground_enter(0);
    if(!terminal){
        via_write(0,14,0x7F);(void)native_motion_stop();
        cs_mac_trace.cause=cause;cs_mac_diag.fault=code;terminal=1u;
    }
    cs_m68k_foreground_leave(0,token);
    (void)cs_foreground_stop(&session.foreground);
    for(;;) { }
}
#if CLASSICK_MAC_FIRST_INPUT_PROBE
/* Kept as an original linked proof point; GC drops it from every normal mode. */
__attribute__((noinline)) int cs_mac_first_input_probe_gate(uint32_t services,
    uint32_t losses,uint32_t deliveries)
{
    return cs_mac_first_input_ready(services,losses,deliveries);
}
#endif
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
        cs_mac_trace.loss_rr3=motion.first_loss.rr3;
        cs_mac_trace.loss_snap[0]=motion.first_loss.snapshots[0];
        cs_mac_trace.loss_snap[1]=motion.first_loss.snapshots[1];
        cs_mac_trace.loss_snap[2]=motion.first_loss.snapshots[2];
        cs_mac_trace.loss_previous=motion.first_loss.previous;
        cs_mac_trace.loss_phase=motion.first_loss.inbox_phase;
        cs_mac_trace.loss_reason=motion.first_loss.reason;
#if CLASSICK_MAC_TIMER_POLL_PROBE && CLASSICK_MAC_TIMER_ACTIVITY_PROBE
        /* Counted services are not a wall-time or input-causality guarantee. */
#if CLASSICK_MAC_TIMER_RECOVERY_PROBE
        int completion=cs_mac_activity_window_step(&cs_mac_activity_gate,motion.services,
            (timer_motion.activity.nonzero_queued || timer_motion.activity.buttononly_queued ||
             timer_motion.activity.losses)?1u:0u);
        if(completion<0){cs_mac_trace.cause=completion;halt(18);}
        if(completion!=CS_MAC_AW_WAIT){
#else
        if ((motion.services >= 500u &&
             (timer_motion.activity.nonzero_queued || timer_motion.activity.losses)) ||
            motion.services >= 50000u) {
#endif
            cs_mac_activity_trace = timer_motion.activity;
            cs_mac_activity_checkpoint();
        }
#elif CLASSICK_MAC_FIRST_INPUT_PROBE
        /* Freeze the copied record before any later UI action; not shutdown. */
        if(cs_mac_first_input_probe_gate(cs_mac_trace.services,cs_mac_trace.losses,
            cs_mac_trace.deliveries))cs_mac_window_idle_checkpoint();
#elif CLASSICK_MAC_IDLE_PROBE
        /* Minimum service count, not a timing promise; trace is already copied. */
        if(motion.services>=500u)cs_mac_window_idle_checkpoint();
#endif
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
    cs_mac_trace.tag=UINT32_C(0x4D575031);cs_mac_trace.version=CLASSICK_MAC_TIMER_PHASE_WITNESS?3u:2u;
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
#if CLASSICK_MAC_TIMER_PAIR_PROBE
    mio.pair=cs_mac_native_pair;
#endif
    r=native_motion_prepare(&mio,&io);
    if(r!=CS_MAC_MOTION_OK){cs_mac_trace.cause=r;halt(12);}
#if CLASSICK_MAC_MOTION_TIMELINE
    cs_mac_timeline_address=(uint32_t)(uintptr_t)&motion.timeline;
#endif
#if CLASSICK_MAC_TIMER_RECOVERY_PROBE
    if(cs_mac_activity_window_begin(&cs_mac_activity_gate)!=0)halt(18);
#endif
    cs_mac_diag.stage=6;
    /* Still IPL7 and quiescent: install level 1/2/3 autovectors. */
    cs_mac_window_install_vectors();
    r=native_motion_arm();
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
