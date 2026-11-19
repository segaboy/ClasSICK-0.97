/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../core/storage/mac-boot-image.h"
#include "../../platform/pc286/boothush-floppy-read.h"
#include "../../platform/pc286/m68k-banked.h"
#include "../../personality/system1/startup-globals.h"

enum { LOAD_REFUSED = 1, SETUP_FAILED, STEP_REFUSED, OUTSIDE, BOUNDARY_POPPED, BUDGET, BOUNDARY };
/* These five words contain only loader status, aggregate steps and logical PCs.
 * Opcode, report operands and private guest backing are never export ranges. */
volatile uint32_t cs_handoff_summary[5];
static cs_mac_boot_image image;
static cs_mac_boot_workspace workspace;
static cs_pc286_floppy provider;
static cs_block_device device;
static cs_pc286_m68k_memory memory;
static cs_m68k_bus_backend backend;
static cs_trap_frame frame;
static cs_m68k_bus_report detail;
static uint8_t low_globals[0x728];

static void post(uint8_t value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];

static int allow(const void *context, const cs_m68k_guard_event *event)
{
    (void)context; (void)event;
    return 1;
}

static int write_span(uint32_t base, const uint8_t *source, unsigned bytes)
{
    while (bytes) {
        unsigned count = bytes < 64 ? bytes : 64;
        if (cs_pc286_guest_write_bytes(&memory.guest, memory.generation, base, count, source) != CS_PG_OK)
            return 0;
        source += count;
        base += count;
        bytes -= count;
    }
    return 1;
}

static int setup(void)
{
    static const cs_pg_region region = {0, 0x20000UL, 7, 0, 4};
    uint32_t slot;
    cs_pg_bank banks[4];
    unsigned i, j;
    for (i = 0; i < 4; ++i) {
        uint16_t segment = (uint16_t)(0x3000u + i * 0x800u);
        volatile uint8_t __far *bank = (volatile uint8_t __far *)((uint32_t)segment << 16);
        banks[i].id = (uint16_t)(i + 1);
        banks[i].segment = segment;
        banks[i].offset = 0;
        banks[i].bytes = 0x8000;
        for (j = 0; j < 0x8000u; ++j)
            bank[j] = 0;
    }
    if (cs_pc286_guest_bind(&memory.guest, &region, 1, banks, 4, 0x30000UL,
            0x20000UL, &memory.generation) != CS_PG_OK ||
        cs_pc286_m68k_backend(&backend, &memory) != CS_PG_OK ||
        !cs_mac128k_seed_startup_globals(low_globals, sizeof low_globals) ||
        !write_span(0x100, low_globals, sizeof low_globals) ||
        !write_span(0x8000, image.blocks, sizeof image.blocks))
        return 0;
    frame.pc = 0x8002;
    frame.a[0] = 0x8000;
    frame.a[7] = 0x77fc;
    frame.sr = 0x2700;
    if (cs_pc286_guest_write(&memory.guest, memory.generation, 0x7ffe, 2, 0x60fe) != CS_PG_OK ||
        cs_pc286_guest_write(&memory.guest, memory.generation, frame.a[7], 4, 0x7ffe) != CS_PG_OK ||
        cs_pc286_guest_read(&memory.guest, memory.generation, frame.a[7], 4, CS_PG_READ, &slot) != CS_PG_OK)
        return 0;
    for (i = 0; i < 8; ++i)
        if (frame.d[i] || (i > 0 && i < 7 && frame.a[i]))
            return 0;
    return slot == 0x7ffe && frame.pc == 0x8002 && frame.sr == 0x2700 &&
        frame.a[0] == 0x8000 && frame.a[7] == 0x77fc;
}

static unsigned execute(void)
{
    static const cs_m68k_stack_limits stack = {0x7000, 0x800};
    static const cs_m68k_guard guard = {allow, NULL, 0};
    unsigned steps;
    if (!setup())
        return SETUP_FAILED;
    cs_handoff_summary[2] = frame.pc;
    for (steps = 0; steps < 256; ++steps) {
        cs_m68k_result r = cs_m68k_bus_step_backend(&backend, &frame, &detail, &stack, &guard);
        if (r != CS_M68K_OK) {
            cs_handoff_summary[4] = (uint32_t)r;
            return STEP_REFUSED;
        }
        cs_handoff_summary[1] = steps + 1u;
        cs_handoff_summary[2] = frame.pc;
        if (steps == 0)
            cs_handoff_summary[3] = frame.pc;
        if (frame.pc == 0x7ffe)
            return frame.a[7] == 0x7800 ? BOUNDARY_POPPED : BOUNDARY;
        if (frame.pc < 0x8000 || frame.pc >= 0x8400)
            return OUTSIDE;
    }
    return BUDGET;
}

void start(void)
{
    unsigned result = LOAD_REFUSED;
    if (cs_pc286_floppy_bind(&provider, 0, 800, &device) &&
        cs_mac_boot_image_load(&device, &workspace, &image) == CS_MAC_BOOT_OK)
        result = execute();
    cs_handoff_summary[0] = result;
    post(0xbd);
    for (;;) { }
}
