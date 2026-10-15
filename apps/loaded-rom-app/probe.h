/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original first loaded ROM application probe. */
#ifndef CLASSICK_LOADED_ROM_APP_PROBE_H
#define CLASSICK_LOADED_ROM_APP_PROBE_H
#include "../../personality/system1/code-run.h"
/* Cold zeroed caller workspace. Every control is a disjoint sibling subobject. */
typedef struct {
    uint8_t low[1024],ram[32768],staging[64];
    cs_guest_region regions[3];cs_guest_map map;cs_mm_manager manager;
    cs_life_controller lifecycle;cs_image_service image;cs_image_workspace image_workspace;
    cs_image_description description;cs_image_request image_request;cs_image_report image_report;
    cs_image_run_request run_request;cs_image_run_report run_report;
    cs_trap_frame caller;cs_life_entry inventory;uint32_t prepared_phase;
} cs_loaded_app_workspace;
typedef struct { uint32_t success,phase,value,pattern,steps,generation,retired; } cs_loaded_app_result;
/* Phases: 1 init, 2 prepared, 3 loaded, 4 returned, 5 result verified, 6 retired.
 * Returns 1 only after full verified retirement; otherwise 0 with last phase kept.
 * Workspace must be cold and zeroed; gateway already initialized. Faults are
 * never retried, reset or scrubbed. */
/* Original demo preparation, including fresh generations after successful
 * retirement. Never resets an active/faulted owner. Used by conformance too. */
int cs_loaded_app_prepare(cs_loaded_app_workspace *w,cs_trap_gateway *gateway);
/* Original negative conformance programs are wrapped in the same resource fork
 * and pass PREPARE/LOAD; no post-LOAD mutation substitutes for loading. */
int cs_loaded_app_prepare_program(cs_loaded_app_workspace *w,cs_trap_gateway *gateway,
    const uint8_t *program,size_t length);
int cs_loaded_app_probe(cs_loaded_app_workspace *w,cs_trap_gateway *gateway,cs_loaded_app_result *result);
#endif
