/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "native.h"
#include "keyboard.h"
#include "uart.h"
#include "../pc/x64/state.h"
typedef void (CS_EFIAPI *cs_native_transfer)(cs_uefi_handoff *,uint64_t);
extern cs_native_transfer const volatile cs_entry_anchor;
extern void CS_EFIAPI cs_native_halt(void);
extern void CS_EFIAPI cs_x64_install(void *state);
extern const unsigned char cs_x64_vector_base[];
extern uint32_t CS_EFIAPI cs_x64_inl(uint32_t port);
extern uint32_t CS_EFIAPI cs_x64_inb(uint32_t port);
extern void CS_EFIAPI cs_x64_outb(uint32_t port,uint32_t value);
static uint8_t keyboard_read(void *context,uint16_t port)
{
    (void)context; return (uint8_t)cs_x64_inb(port);
}
static void keyboard_write(void *context,uint16_t port,uint8_t value)
{
    (void)context; cs_x64_outb(port,value);
}
static uint32_t port32(void *context,uint16_t port)
{
    (void)context;
    return cs_x64_inl(port);
}
void CS_EFIAPI cs_native_stop(cs_uefi_handoff *handoff)
{
    volatile uint32_t *entered=&handoff->native_entered;
    *entered=1;
    cs_x64_layout layout;
    layout.state_base=handoff->bundle_base+CS_LOADER_CPU_OFFSET;
    layout.vector_base=(uint64_t)(uintptr_t)cs_x64_vector_base;
    layout.stack_top=handoff->stack_top;
    for(unsigned i=0;i<4;++i)
        layout.ist_top[i]=handoff->bundle_base+CS_LOADER_FAULT_OFFSET+(i+1)*UINT64_C(4096);
    if(cs_x64_tables_init((void *)(uintptr_t)layout.state_base,CS_X64_STATE_BYTES,&layout)==CS_X64_OK)
        cs_x64_install((void *)(uintptr_t)layout.state_base);
    /* SPEC-0009: presentation itself rechecks successful exit and the ready marker. */
    volatile uint32_t *ready=(volatile uint32_t *)(uintptr_t)(layout.state_base+CS_X64_READY);
    (void)cs_native_present(handoff,*ready,
        (volatile unsigned char *)(uintptr_t)handoff->framebuffer.base,
        (void *)(uintptr_t)handoff->arena_base,(unsigned char *)(uintptr_t)handoff->trace_base);
    /* SPEC-0010: identity window over the profile's physical range; map-type checked. */
    cs_native_memory memory;
    memory.map=(const unsigned char *)(uintptr_t)handoff->map_base;
    memory.map_size=(size_t)handoff->map_size; memory.map_stride=(size_t)handoff->map_stride;
    memory.map_version=handoff->map_version;
    memory.window_physical=0; memory.window_size=UINT64_C(1)<<47; memory.window_base=0;
    (void)cs_native_timer_probe(handoff,*ready,&memory,port32,NULL,
        (unsigned char *)(uintptr_t)(handoff->trace_base+CS_NATIVE_TRACE_BYTES));
    /* SPEC-0013: best-effort bounded serial diagnostics alongside SPEC-0012. */
    cs_native_devices devices;
    devices.memory=&memory; devices.port=port32; devices.port_context=NULL;
    devices.framebuffer=(volatile unsigned char *)(uintptr_t)handoff->framebuffer.base;
    devices.arena=(void *)(uintptr_t)handoff->arena_base;
    cs_ps2_io keyboard;
    keyboard.read=keyboard_read; keyboard.write=keyboard_write; keyboard.context=NULL;
    cs_uart_io serial;
    serial.read=keyboard_read; serial.write=keyboard_write; serial.context=NULL;
    /* SPEC-0016: qualification record and probe cell are identity views at 1024/1152. */
    (void)cs_native_uart_loop(handoff,*ready,&devices,&keyboard,&serial,0x3F8,12,60,
        (unsigned char *)(uintptr_t)(handoff->trace_base+CS_KBD_TRACE_OFFSET),
        (unsigned char *)(uintptr_t)(handoff->trace_base+CS_UART_TRACE_OFFSET),
        (unsigned char *)(uintptr_t)(handoff->trace_base+CS_QUAL_TRACE_OFFSET));
    cs_native_halt();
}
cs_efi_status CS_EFIAPI cs_uefi_entry(void *image,cs_efi_system *system)
{
    cs_uefi_load_result result;
    cs_efi_status status=cs_uefi_loader_run(image,system,&result);
    if(result.attempts!=0) {
        cs_entry_anchor(result.handoff,result.handoff->stack_top);
        cs_native_halt();
        for(;;) { }
    }
    return status;
}
