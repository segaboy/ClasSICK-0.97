/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "loader.h"
typedef void (CS_EFIAPI *cs_native_transfer)(cs_uefi_handoff *,uint64_t);
extern cs_native_transfer const volatile cs_entry_anchor;
extern void CS_EFIAPI cs_native_halt(void);
void CS_EFIAPI cs_native_stop(cs_uefi_handoff *handoff)
{
    volatile uint32_t *entered=&handoff->native_entered;
    *entered=1;
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
