/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "rom.h"
#include "../../apps/service-probe/probe.h"
#include "../../core/input/mouse.h"

volatile cs_mac_diagnostics cs_mac_diag;
static cs_trap_gateway cs_mac_services;
_Static_assert(offsetof(cs_mac_diagnostics,traps)==16,"Trap assembly offset");
_Static_assert(offsetof(cs_mac_diagnostics,fault)==24,"Fault assembly offset");
extern void cs_mac_probe(void);
extern void cs_mac_fault(void);
_Static_assert(offsetof(cs_trap_frame,a[7])==60 && offsetof(cs_trap_frame,pc)==64 &&
    offsetof(cs_trap_frame,sr)==68,"Native Line-A frame offsets");
static void aline_fault(void) {cs_mac_diag.fault=7;for(;;) { }}
void cs_mac_line_a(cs_trap_frame *f)
{
    /* Native C workload has no guest ABI/trap entry. The legacy --drawing
     * profile retains its independently tested Line-A/interpreter boundary. */
    (void)f;aline_fault();
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

#include "../../apps/native-drawing/workload.h"
#include "../../apps/product-scene/scene.h"
static cs_qd_rect_port port __attribute__((section(".loaded_app")));
cs_native_draw_result cs_mac_native_result;
static int native_scene(const cs_surface *screen)
{
    cs_product_state state;
    state.value=cs_mac_native_result.value;state.inputs=0;state.progress=0;
    state.phases=cs_mac_native_result.phase;state.success=cs_mac_native_result.success;
    state.pixels=port.pixels;
    return cs_product_scene(screen,512,342,0,&state);
}
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
    cs_mac_via_init(&io);
    if(cs_surface_init(&screen,(uint8_t *)(uintptr_t)0x1A700,CS_MAC_SCREEN_BYTES,
        512,342,64,CS_SURFACE_MONO1_MSB)!=CS_SURFACE_OK){cs_mac_diag.fault=3;for(;;){}}
    uint8_t *bytes=(uint8_t *)&port;for(size_t i=0;i<sizeof port;++i)bytes[i]=0;
    (void)native_scene(&screen);
    int success=cs_native_draw(&port,&cs_mac_native_result);
    cs_mac_diag.stage=success?16u:8u;cs_mac_diag.fault=success?0u:8u;
    if(!native_scene(&screen)){cs_mac_diag.fault=4;for(;;){}}
    cs_mac_loop_begin(&loop,read_reg(0,0));
    cs_mouse_button mouse;
    if(cs_mouse_button_begin(&mouse,loop.mouse_down)!=CS_MOUSE_OK){cs_mac_diag.fault=9;for(;;){}}
    uint32_t shown_presses=0;
    for(;;){
        (void)cs_mac_loop_poll(&loop,&io);cs_mac_diag.ticks=loop.ticks;
        if(cs_mouse_button_sample(&mouse,loop.mouse_down)>CS_MOUSE_CHANGED){cs_mac_diag.fault=10;for(;;){}}
        cs_mac_diag.presses=mouse.presses;
        if(mouse.presses!=shown_presses){
            if(!cs_product_scene_input(&screen,mouse.presses)){cs_mac_diag.fault=5;for(;;){}}
            shown_presses=mouse.presses;
        }
    }
}
