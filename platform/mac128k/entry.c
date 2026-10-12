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
        (f->a[7]&1u)!=0 || f->a[7]<0x2100u || f->a[7]>0x4000u) aline_fault();
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

void cs_mac_main(void)
{
    cs_surface screen;
    cs_mac_loop loop;
    cs_mac_io io;
    io.context=NULL; io.read=read_reg; io.write=write_reg;
    cs_mac_diag.magic=UINT32_C(0x43533039);
    cs_mac_diag.stage=1;
    cs_mac_diag.image_crc=cs_mac_image_crc((const volatile uint8_t *)(uintptr_t)UINT32_C(0x400000),CS_MAC_ROM_BYTES);
    if(cs_mac_diag.image_crc!=*(const volatile uint32_t *)(uintptr_t)UINT32_C(0x400100)) {
        cs_mac_diag.fault=1; for(;;) { }
    }
    cs_mac_probe();
    if(cs_mac_diag.traps!=1u) {cs_mac_diag.fault=2; for(;;) { }}
    cs_service_probe_result service_result;
    int services_ok=cs_service_probe(&cs_mac_services,&service_result);
    cs_mac_diag.service_checks=service_result.checks;
    cs_mac_diag.service_status=service_result.status;
    cs_mac_diag.service_signature=service_result.signature;
    if(!services_ok) {cs_mac_diag.fault=6; for(;;) { }}
    cs_aline_probe_result aline_result;
    if(!cs_aline_probe(&cs_mac_services,&aline_result)) aline_fault();
    cs_mac_diag.aline_checks=aline_result.checks;cs_mac_diag.aline_status=aline_result.status;
    cs_mac_diag.aline_signature=aline_result.signature;cs_mac_diag.aline_steps=aline_result.steps;
    cs_mac_diag.aline_calls=aline_result.calls;
    /* The hardware Set must install this entry itself, independently of the interpreter. */
    if(cs_trap_set(&cs_mac_services,0x20,cs_mac_services.config.unsupported_token)!=CS_TRAP_OK) aline_fault();
    if(!cs_mac_aline_probe() || cs_mac_diag.native_aline_calls!=3) aline_fault();
    cs_mac_via_init(&io);
    if(cs_surface_init(&screen,(uint8_t *)(uintptr_t)UINT32_C(0x1A700),CS_MAC_SCREEN_BYTES,
            512,342,64,CS_SURFACE_MONO1_MSB)!=CS_SURFACE_OK) {
        cs_mac_diag.fault=3; for(;;) { }
    }
    cs_mac_loop_begin(&loop,read_reg(NULL,0));
    cs_mac_diag.stage=2;
    if(!cs_mac_scene(&screen,0,0)) {cs_mac_diag.fault=4; for(;;) { }}
    cs_mac_diag.stage=3;
    for(;;) {
        int changed=cs_mac_loop_poll(&loop,&io);
        cs_mac_diag.ticks=loop.ticks; cs_mac_diag.presses=loop.presses;
        if(changed && !cs_mac_scene(&screen,loop.ticks,loop.presses)) {
            cs_mac_diag.fault=5; for(;;) { }
        }
        if(loop.ticks==CS_MAC_TOTAL_TICKS) cs_mac_diag.stage=4;
    }
}
