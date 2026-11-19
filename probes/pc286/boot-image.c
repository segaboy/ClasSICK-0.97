/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../core/storage/mac-boot-image.h"
#include "../../platform/pc286/boothush-floppy-read.h"
#include "../../platform/pc286/m68k-banked.h"

volatile uint32_t cs_boot_report[4];
cs_mac_boot_image cs_boot_image;
static cs_mac_boot_workspace workspace;
static cs_pc286_floppy provider;
static cs_block_device device;
static cs_pc286_m68k_memory guest;
static cs_m68k_bus_backend backend;
static cs_trap_frame frame;
static cs_m68k_bus_report report;
static void post(uint8_t value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];

static int allow(const void *context, const cs_m68k_guard_event *event)
{
    (void)context; (void)event;
    return 1;
}

static int execute(void)
{
    /* The MOVE instruction spans sectors. Its four-byte immediate operand
     * starts in sector two and spans the two guest banks. */
    static const cs_pg_region region = {0x8000, 2048, 7, 0, 2};
    static const cs_pg_bank banks[] = {{1, 0x3000, 0, 513}, {2, 0x3800, 0, 1535}};
    static const cs_m68k_stack_limits limits = {0x8600, 512};
    static const cs_m68k_guard guard = {allow, NULL, 0};
    unsigned offset, step;
    if (cs_pc286_guest_bind(&guest.guest, &region, 1, banks, 2, 0x30000UL,
            0x10000UL, &guest.generation) != CS_PG_OK ||
        cs_pc286_m68k_backend(&backend, &guest) != CS_PG_OK)
        return 0;
    for (offset = 0; offset < sizeof cs_boot_image.blocks; offset += 64)
        if (cs_pc286_guest_write_bytes(&guest.guest, guest.generation,
                0x8000UL + offset, 64, cs_boot_image.blocks + offset) != CS_PG_OK)
            return 0;
    frame.pc = 0x8002;
    frame.sr = 0x2700;
    frame.a[7] = 0x87f8;
    if (cs_pc286_guest_write(&guest.guest, guest.generation, frame.a[7],
            4, 0x85fe) != CS_PG_OK)
        return 0;
    for (step = 0; step < 3; ++step)
        if (cs_m68k_bus_step_backend(&backend, &frame, &report, &limits, &guard) != CS_M68K_OK)
            return 0;
    if (frame.pc != 0x85fe || frame.a[7] != 0x87fc)
        return 0;
    cs_boot_report[2] = frame.d[0];
    return 1;
}

void start(void)
{
    cs_mac_boot_result result = CS_MAC_BOOT_ARGUMENT;
    if (cs_pc286_floppy_bind(&provider, 0, 800, &device))
        result = cs_mac_boot_image_load(&device, &workspace, &cs_boot_image);
    cs_boot_report[1] = (uint32_t)result;
    if (result == CS_MAC_BOOT_OK)
        cs_boot_report[3] = (uint32_t)execute();
    cs_boot_report[0] = 0xb1;
    post(0xb1);
    for (;;) { }
}
