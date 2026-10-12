/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "rom.h"
#include "../../apps/service-probe/probe.h"

volatile cs_mac_diagnostics cs_mac_diag;
static cs_trap_gateway cs_mac_services;
_Static_assert(offsetof(cs_mac_diagnostics,traps)==16,"Trap assembly offset");
_Static_assert(offsetof(cs_mac_diagnostics,fault)==24,"Fault assembly offset");
extern void cs_mac_probe(void);
extern void cs_mac_fault(void);

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
