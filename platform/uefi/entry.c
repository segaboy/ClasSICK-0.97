/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "native.h"
#include "../pc/x64/state.h"
typedef void (CS_EFIAPI *cs_native_transfer)(cs_uefi_handoff *,uint64_t);
extern cs_native_transfer const volatile cs_entry_anchor;
extern void CS_EFIAPI cs_native_halt(void);
extern void CS_EFIAPI cs_x64_install(void *state);
extern const unsigned char cs_x64_vector_base[];
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
