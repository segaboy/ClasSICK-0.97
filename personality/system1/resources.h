/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_RESOURCES_H
#define CLASSICK_SYSTEM1_RESOURCES_H
#include "../../core/memory/lifecycle.h"
#include "../../formats/resourcefork/reader.h"
#define CS_RES_CACHE 16u
#define CS_RES_TICKETS 32u
typedef enum { CS_RES_OK=0,CS_RES_ARGUMENT=1,CS_RES_STATE=2,CS_RES_CONTEXT=3,
    CS_RES_SOURCE=4,CS_RES_NOT_FOUND=5,CS_RES_AMBIGUOUS=6,CS_RES_UNRESOLVED_REFERENCE=7,
    CS_RES_SYSTEM_HEAP_UNSUPPORTED=8,CS_RES_PROFILE_LIMIT=9,CS_RES_CACHE_FULL=10,
    CS_RES_TICKETS_FULL=11,CS_RES_MANAGER_CAPACITY=12,CS_RES_COUNTER_EXHAUSTED=13,
    CS_RES_INVALID_TICKET=14,CS_RES_NOT_RESIDENT=15,CS_RES_IN_USE=16,
    CS_RES_CONFLICT=17,CS_RES_FAULT=18 } cs_res_result;
typedef enum { CS_RES_CLOSED=0,CS_RES_OPENED=1,CS_RES_BUSY=2,CS_RES_FAULTED=3 } cs_res_state;
typedef enum { CS_RES_OPEN=1,CS_RES_ACQUIRE=2,CS_RES_DESCRIBE=3,CS_RES_RELEASE=4,
    CS_RES_EVICT=5,CS_RES_CLOSE=6 } cs_res_operation;
typedef enum { CS_RES_PREFLIGHT=1,CS_RES_LOOKUP=2,CS_RES_ALLOCATION=3,CS_RES_LOCKING=4,
    CS_RES_COPYING=5,CS_RES_CACHE_PUBLICATION=6,CS_RES_TICKET_PUBLICATION=7,
    CS_RES_DISPOSAL=8,CS_RES_RETIREMENT=9,CS_RES_OWNERSHIP=10,CS_RES_DONE=11 } cs_res_phase;
typedef struct { uint32_t maximum_source,maximum_resource,owner; } cs_res_profile;
typedef struct { cs_life_context context;
    uint32_t active_root,active_callback,active_patch,active_manager;
    const cs_m68k_session *session;
    /* Truthful acknowledgment of the coordinator's current authoritative entry. */
    const cs_life_entry *registration;
} cs_res_context;
struct cs_res_service;
typedef struct { const struct cs_res_service *owner;
    uint32_t generation,epoch,identity,slot,type;int32_t id;uint32_t handle,length;
} cs_res_ticket;
typedef struct { cs_res_operation operation;cs_res_context context;
    const uint8_t *source;size_t length;uint32_t type;int32_t id;
    const cs_res_ticket *ticket;
} cs_res_request;
typedef struct { cs_res_result cause;cs_res_operation operation;cs_res_phase phase;cs_res_state state;
    uint32_t generation,generation_valid,epoch,epoch_valid,heap_epoch,heap_epoch_valid,retained_fault;
    uint32_t reader_valid,entry_valid,lifecycle_valid,manager_valid,reply_valid;
    cs_rf_report reader;cs_rf_entry entry;cs_life_result lifecycle_result;cs_mm_result manager_result;cs_mm_reply reply;
    uint32_t guest_committed,native_committed,completed,allocated,moved,handle_valid,handle;
    uint32_t cache_valid,ticket_valid,span_valid,empty,address,length;
    cs_res_ticket ticket;cs_guest_view span;uint16_t disposed_mask,retired_mask;
} cs_res_report;
typedef struct { uint32_t live,leases,manager_slot,handle,address,length,extent;
    cs_rf_entry entry;cs_rf_report reader;
} cs_res_cached;
typedef struct { uint32_t live;cs_res_ticket ticket; } cs_res_lease;
typedef struct cs_res_service { uint32_t tag;const struct cs_res_service *self;
    cs_mm_manager *manager;cs_life_controller *lifecycle;const cs_guest_map *map;
    cs_mm_config config;size_t region_count;const cs_guest_region *region_descriptors;
    cs_guest_region regions[CS_GUEST_MAX_REGIONS];
    cs_res_profile profile;cs_res_state state;
    uint32_t last_open_generation,last_ticket_id,generation,epoch,source_borrow;
    const uint8_t *source;size_t source_length;cs_rf_view view;
    cs_res_cached cache[CS_RES_CACHE];cs_res_lease leases[CS_RES_TICKETS];
    cs_res_report first_fault;
#ifdef CLASSICK_RESOURCES_TEST_FAULTS
    uint32_t test_after,test_call,test_call_index;
#endif
} cs_res_service;
/* SPEC-0031. Service starts zeroed. All native objects are aligned, truthful,
 * synchronized and disjoint from each other, sources and guest backing bytes.
 * Tagged services cannot be reinitialized. No host allocation or callbacks.
 * Source is immutable until successful CLOSE return; FAULT retains its borrow.
 * Independent mutation of owned handles/payloads and controls is unsupported.
 * The failing call's ticket/span validity is diagnostic in FAULT. Later calls
 * return fresh no-effect FAULT refusals; the first_fault snapshot stays owned
 * and may be inspected without creating a data span or usable borrow. */
cs_res_result cs_res_init(cs_res_service *s,cs_life_controller *lifecycle,const cs_res_profile *profile);
cs_res_result cs_res_execute(cs_res_service *s,const cs_res_request *q,cs_res_report *out);
/* Composite coordinator gate for execution/reset: CLOSED only. All entries
 * still require truthful fresh inventory and the applicable lifecycle gate.
 * Low-level manager/session/lifecycle APIs are not intercepted. */
cs_res_result cs_res_external_admit(const cs_res_service *s);
#ifdef CLASSICK_RESOURCES_TEST_FAULTS
/* Test-only post-unit failure, or unexpected native manager failure before
 * call 1=allocation,2=lock,3=disposal at the operation's one-based call index.
 * Kinds4/5 corrupt address/extent after locking (index=1), then exercise fresh
 * internal invariant validation. Zero disables each seam. No external function
 * pointer or production symbol; before-call seams publish no native result. */
cs_res_result cs_res_test_fault(cs_res_service *s,uint32_t after,uint32_t call,uint32_t index);
#endif
#endif
