/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MEMORY_LIFECYCLE_H
#define CLASSICK_MEMORY_LIFECYCLE_H
#include "manager.h"
#include "../cpu/m68k-session.h"
#define CS_LIFE_INVENTORY 64u
typedef enum { CS_LIFE_OK=0,CS_LIFE_ARGUMENT=1,CS_LIFE_STATE=2,
    CS_LIFE_CONTEXT=3,CS_LIFE_MEMORY=4,CS_LIFE_ALIAS=5,CS_LIFE_REFERENCE=6,
    CS_LIFE_SCRAP=7,CS_LIFE_CAPACITY=8,CS_LIFE_EPOCH=9,CS_LIFE_CHANGED=10,
    CS_LIFE_INVARIANT=11,CS_LIFE_FAULT=12 } cs_life_result;
typedef enum { CS_LIFE_STABLE=0,CS_LIFE_PREPARED=1,CS_LIFE_COMMITTING=2,
    CS_LIFE_POPULATING=3,CS_LIFE_FAULTED=4 } cs_life_state;
typedef enum { CS_LIFE_RESET=1,CS_LIFE_TEARDOWN=2 } cs_life_operation;
typedef enum { CS_LIFE_UNKNOWN_CONTEXT=0,CS_LIFE_ORDINARY=1,CS_LIFE_INTERRUPT=2 } cs_life_context;
typedef enum { CS_LIFE_NONE=0,CS_LIFE_RESIDENT=1,CS_LIFE_DISK_UNKNOWN=2 } cs_life_residency;
typedef enum { CS_LIFE_GLOBAL=1,CS_LIFE_NATIVE=2,CS_LIFE_GUEST=3,
    CS_LIFE_BUILTIN=4,CS_LIFE_FUTURE=5 } cs_life_kind;
typedef enum { CS_LIFE_PRESERVE=1,CS_LIFE_REBIND=2,CS_LIFE_CLEAR=3,CS_LIFE_REFUSE=4 } cs_life_action;
typedef enum { CS_LIFE_PREFLIGHT=1,CS_LIFE_STAGING=2,CS_LIFE_COMMIT=3,
    CS_LIFE_POPULATION=4,CS_LIFE_SCRAP_PRESERVE=5,CS_LIFE_DONE=6 } cs_life_phase;
typedef struct { int32_t size;cs_life_residency residency;
    uint32_t handle,name_address,name_bytes; } cs_life_scrap;
/* GLOBAL: address/bytes/value are a current one of the six manager views.
 * NATIVE/GUEST: opaque owned immutable control/input bytes, no retained heap
 * pointer semantics. BUILTIN: address is slot, value is a separately trusted
 * immutable owned token, bytes=4. It must not be inferred from the live table.
 * FUTURE/CLEAR/REFUSE registrations are unsupported. owner must be nonzero. */
typedef struct { cs_life_kind kind;cs_life_action action;
    uint32_t owner,address,bytes,value;const void *native; } cs_life_entry;
typedef struct {
    cs_life_operation operation;cs_life_context context;
    uint32_t active_root,active_callback,active_patch,active_manager;
    uint32_t stack_base,stack_size,stack_reference,reserve,minimum;
    cs_life_scrap *scrap;uint8_t *staging;size_t staging_bytes;
    const cs_life_entry *inventory;size_t count;
    cs_trap_gateway *gateway;const cs_m68k_session *session;
} cs_life_request;
typedef struct { cs_life_result cause;cs_life_operation operation;cs_life_phase phase;
    uint32_t prepared,committed,epoch_valid,epoch_before,epoch_after;
    uint32_t attempted,completed,scrap_valid,old_handle,new_handle,new_handle_valid,size;
    uint32_t globals_valid,globals[6];cs_life_state state; } cs_life_report;
struct cs_life_plan;
typedef struct { uint32_t tag,epoch;cs_life_state state;cs_mm_manager *manager;
    struct cs_life_plan *active;cs_life_report first_fault;
#ifdef CLASSICK_LIFECYCLE_TEST_FAULTS
    uint32_t test_fault_after;
#endif
} cs_life_controller;
typedef struct cs_life_plan {
    uint32_t tag,ready;cs_life_controller *owner;const cs_life_request *input;
    cs_life_request request;cs_life_scrap scrap;
    cs_mm_manager manager;cs_guest_map map;
    cs_guest_region regions[CS_GUEST_MAX_REGIONS];
    cs_life_entry inventory[CS_LIFE_INVENTORY];
    uint32_t globals[6],cells[2][CS_MM_BLOCKS];
    cs_trap_config gateway_config;uint32_t gateway_magic,entries[CS_TRAP_ENTRIES];
    cs_guest_view zones[2],global_views[6],source;
    uint32_t epoch,limit,slot,address,extent,app_scrap,retire_mask,write_units;
#ifdef CLASSICK_LIFECYCLE_TEST_FAULTS
    uint32_t test_fault_after;
#endif
} cs_life_plan;
/* SPEC-0030 v2. Native storage is truthful/live/aligned, caller synchronized.
 * Controller and each new plan start zeroed; tagged controllers cannot be reinitialized. All
 * native objects and borrowed buffers are disjoint. Plans are owned protocol
 * state, not untrusted serializable input; mutation/cloning is not authorized.
 * Prepared opaque protected contents must remain immutable by the caller.
 * All meaningful request/map/config/record/cell/global/gateway values and staged
 * scrap are compared at commit. No full RAM snapshot or native authentication. */
cs_life_result cs_life_init(cs_life_controller *c,cs_mm_manager *manager);
cs_life_result cs_life_prepare(cs_life_controller *c,const cs_life_request *q,
    cs_life_plan *p,cs_life_report *out);
cs_life_result cs_life_commit(cs_life_controller *c,cs_life_plan *p,cs_life_report *out);
cs_life_result cs_life_cancel(cs_life_controller *c,cs_life_plan *p,cs_life_report *out);
/* Trusted coordinated entry gate. Independent low-level APIs are not intercepted.
 * Before each coordinated manager/session operation the host enforces this gate
 * and the same closed-world admission/lifetime preconditions. */
cs_life_result cs_life_admit(const cs_life_controller *c);
cs_life_result cs_life_manager_call(cs_life_controller *c,const cs_mm_request *q,
    cs_mm_reply *out,cs_mm_result *manager_result);
#ifdef CLASSICK_LIFECYCLE_TEST_FAULTS
/* Conformance build only: post-write internal invariant injection, no callback. */
cs_life_result cs_life_test_fault_after(cs_life_controller *c,uint32_t writes);
#endif
#endif
