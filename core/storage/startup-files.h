/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CORE_STORAGE_STARTUP_FILES_H
#define CLASSICK_CORE_STORAGE_STARTUP_FILES_H
#include "mac-boot-header.h"
#include "../../formats/resourcefork/mfs-transport.h"

#define CS_STARTUP_FILES 2u
enum { CS_STARTUP_SYSTEM=0,CS_STARTUP_SHELL=1,CS_STARTUP_NO_FILE=2 };
typedef enum { CS_STARTUP_OK=0,CS_STARTUP_ARGUMENT=1,CS_STARTUP_REFUSED=2 } cs_startup_result;
typedef enum {
    CS_STARTUP_HEADER=1,CS_STARTUP_MOUNT=2,CS_STARTUP_FIND=3,
    CS_STARTUP_BIND=4,CS_STARTUP_OPEN=5,CS_STARTUP_COMPLETE=6
} cs_startup_stage;
typedef enum { CS_STARTUP_NONE=0,CS_STARTUP_BOOT=1,CS_STARTUP_MFS=2,
    CS_STARTUP_RESOURCE=3 } cs_startup_domain;
typedef struct {
    cs_startup_stage stage;unsigned file;cs_startup_domain domain;
    unsigned status;cs_rf_field field;uint32_t position;
    uint32_t opened,types,references,warnings;
} cs_startup_report;
typedef struct {
    cs_mac_boot_header header;
    cs_mfsv_volume volume;
    cs_mfs_entry files[CS_STARTUP_FILES];
    cs_rfs_mfs_adapter adapters[CS_STARTUP_FILES];
    cs_rfs_view resources[CS_STARTUP_FILES];
} cs_startup_files;
typedef struct {
    cs_startup_files candidate;
    cs_mac_boot_workspace boot;
    cs_mfsv_workspace disk;
    cs_rfs_workspace resource;
} cs_startup_workspace;

/* SPEC-0122. Acquire header, mount the same immutable device, find the first
 * exact counted System/Shell names, and open both resource maps. Any failure
 * preserves every byte of out. Argument refusal also preserves report/scratch.
 * Other refusals may change report/workspace/provider state. On success out
 * owns its controls; resources[i].source is immediately usable. Its callbacks
 * borrow scratch->disk. Keep out at its original address and all device media,
 * provider context and scratch live/exclusive while using either source. Do
 * not copy/move out, recurse, interleave accesses, or mutate retained controls.
 * Native objects are truthful, disjoint and confined to individual segments;
 * exact top-level aliases are checked, arbitrary overlaps are preconditions.
 * No code/entry interpretation, INIT dispatch, resource payload read, guest
 * pointer, allocation, OS trap, memory sizing or filename diagnostic export. */
cs_startup_result cs_startup_files_open(const cs_block_device *device,
    cs_startup_workspace *scratch,cs_startup_files *out,cs_startup_report *report);
#endif
