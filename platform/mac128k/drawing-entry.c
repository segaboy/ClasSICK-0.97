/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "rom.h"
#include "../../apps/service-probe/probe.h"
#include "../../apps/aline-probe/probe.h"

volatile cs_mac_diagnostics cs_mac_diag;
static cs_trap_gateway cs_mac_services;
_Static_assert(offsetof(cs_mac_diagnostics,traps)==16,"Trap assembly offset");
_Static_assert(offsetof(cs_mac_diagnostics,fault)==24,"Fault assembly offset");
extern void cs_mac_probe(void);
extern void cs_mac_fault(void);
extern int cs_mac_aline_probe(void);
extern const uint8_t cs_mac_aline_program[],cs_mac_aline_program_end[];
_Static_assert(offsetof(cs_trap_frame,a[7])==60 && offsetof(cs_trap_frame,pc)==64 &&
    offsetof(cs_trap_frame,sr)==68,"Native Line-A frame offsets");
static void aline_fault(void) {cs_mac_diag.fault=7;for(;;) { }}
void cs_mac_line_a(cs_trap_frame *f)
{
    uint32_t base=(uint32_t)(uintptr_t)cs_mac_aline_program;
    uint32_t end=(uint32_t)(uintptr_t)cs_mac_aline_program_end;
    cs_m68k_report report;
    if((f->sr&0xA000u)!=0x2000u || (f->pc&1u)!=0 || f->pc<base || f->pc>=end ||
        (f->a[7]&1u)!=0 || f->a[7]<0x2100u || f->a[7]>0x8000u) aline_fault();
    const uint8_t *p=(const uint8_t *)(uintptr_t)f->pc;
    uint16_t word=(uint16_t)(((uint16_t)p[0]<<8)|p[1]);
    if(cs_m68k_aline(&cs_mac_services,word,f,&report)!=CS_M68K_OK) aline_fault();
    ++cs_mac_diag.native_aline_calls;
}

static uint8_t read_reg(void *context,unsigned reg)
{
    (void)context;
    return *(volatile uint8_t *)(uintptr_t)(UINT32_C(0xEFE1FE)+reg*512u);
}
static void write_reg(void *context,unsigned reg,uint8_t value)
{
    (void)context;
    *(volatile uint8_t *)(uintptr_t)(UINT32_C(0xEFE1FE)+reg*512u)=value;
}

#include "../../apps/drawing-rom-app/probe.h"
#include "../../apps/loaded-rom-app/scene.h"
static cs_drawing_app_workspace workspace __attribute__((section(".loaded_app")));
cs_loaded_app_result cs_mac_loaded_result;
void cs_mac_main(void)
{
    cs_surface screen;cs_mac_loop loop;cs_mac_io io;
    io.context=0;io.read=read_reg;io.write=write_reg;
    cs_mac_diag.magic=UINT32_C(0x43533039);cs_mac_diag.stage=1;
    cs_mac_diag.image_crc=cs_mac_image_crc((const volatile uint8_t *)(uintptr_t)0x400000,CS_MAC_ROM_BYTES);
    if(cs_mac_diag.image_crc!=*(const volatile uint32_t *)(uintptr_t)0x400100){cs_mac_diag.fault=1;for(;;){}}
    cs_mac_probe();if(cs_mac_diag.traps!=1){cs_mac_diag.fault=2;for(;;){}}
    cs_service_probe_result service;
    if(!cs_service_probe(&cs_mac_services,&service)){cs_mac_diag.fault=6;for(;;){}}
    cs_aline_probe_result aline;
    if(!cs_aline_probe(&cs_mac_services,&aline))aline_fault();
    if(cs_trap_set(&cs_mac_services,0x20,cs_mac_services.config.unsupported_token)!=CS_TRAP_OK)aline_fault();
    if(!cs_mac_aline_probe() || cs_mac_diag.native_aline_calls!=3)aline_fault();
    cs_mac_via_init(&io);
    if(cs_surface_init(&screen,(uint8_t *)(uintptr_t)0x1A700,CS_MAC_SCREEN_BYTES,
        512,342,64,CS_SURFACE_MONO1_MSB)!=CS_SURFACE_OK){cs_mac_diag.fault=3;for(;;){}}
    uint8_t *bytes=(uint8_t *)&workspace;for(size_t i=0;i<sizeof workspace;++i)bytes[i]=0;
    (void)cs_drawing_app_scene(&screen,&cs_mac_loaded_result,&workspace.port,0);
    int success=cs_drawing_app_probe(&workspace,&cs_mac_services,&cs_mac_loaded_result);
    cs_mac_diag.stage=success?16u:8u;cs_mac_diag.fault=success?0u:8u;
    if(!cs_drawing_app_scene(&screen,&cs_mac_loaded_result,&workspace.port,0)){cs_mac_diag.fault=4;for(;;){}}
    cs_mac_loop_begin(&loop,read_reg(0,0));
    uint32_t shown_presses=0;
    for(;;){
        (void)cs_mac_loop_poll(&loop,&io);cs_mac_diag.ticks=loop.ticks;cs_mac_diag.presses=loop.presses;
        if(loop.presses!=shown_presses){
            if(!cs_loaded_app_scene_input(&screen,loop.presses)){cs_mac_diag.fault=5;for(;;){}}
            shown_presses=loop.presses;
        }
    }
}
