/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original host-only SPEC-0033 protocol. */
#ifndef CLASSICK_SYSTEM1_CODE_IMAGE_H
#define CLASSICK_SYSTEM1_CODE_IMAGE_H
#include "code-preflight.h"
#include "resources.h"
#define CS_IMAGE_RESOURCES 8u
typedef enum { CS_IMAGE_OK=0,CS_IMAGE_ARGUMENT=1,CS_IMAGE_STATE=2,CS_IMAGE_CONTEXT=3,
    CS_IMAGE_CHANGED=4,CS_IMAGE_CODE=5,CS_IMAGE_PROFILE_LIMIT=6,CS_IMAGE_CAPACITY=7,
    CS_IMAGE_COUNTER_EXHAUSTED=8,CS_IMAGE_IN_USE=9,CS_IMAGE_FAULT=10 } cs_image_result;
typedef enum { CS_IMAGE_EMPTY=0,CS_IMAGE_PREPARED=1,CS_IMAGE_POPULATING=2,
    CS_IMAGE_PARTIAL=3,CS_IMAGE_LOADED=4,CS_IMAGE_RETIRING=5,CS_IMAGE_FAULTED=6 } cs_image_state;
typedef enum { CS_IMAGE_PREPARE=1,CS_IMAGE_LOAD=2,CS_IMAGE_CANCEL=3,
    CS_IMAGE_DESCRIBE=4,CS_IMAGE_RELEASE=5 } cs_image_operation;
typedef enum { CS_IMAGE_ADMISSION=1,CS_IMAGE_SOURCE=2,CS_IMAGE_LAYOUT=3,CS_IMAGE_STAGING=4,
    CS_IMAGE_ALLOCATION=5,CS_IMAGE_LOCKING=6,CS_IMAGE_COPYING=7,CS_IMAGE_WORLD=8,
    CS_IMAGE_TABLE=9,CS_IMAGE_DISPOSAL=10,CS_IMAGE_RETIREMENT=11,CS_IMAGE_DONE=12 } cs_image_phase;
typedef struct { uint32_t maximum_source,maximum_staging,maximum_world,owner; } cs_image_profile;
typedef struct { cs_life_context context;uint32_t active_root,active_callback,active_patch,active_manager;
    const cs_m68k_session *session;const cs_life_entry *inventory;size_t count;
    const cs_res_service *const *resources;size_t resource_count;
} cs_image_context;
typedef struct { cs_image_operation operation;cs_image_context context;
    uint32_t generation,epoch,b,s,h_min;const uint8_t *source;size_t length;
    uint8_t *staging;size_t staging_capacity;
} cs_image_request;
typedef struct { uint32_t handle,address,length,extent,slot; } cs_image_segment;
typedef struct { cs_image_result cause;cs_image_operation operation;cs_image_phase phase;cs_image_state state;
    uint32_t generation,generation_valid,epoch,epoch_valid,heap_epoch,heap_epoch_valid,retained_fault;
    uint32_t preflight_valid;cs_code_report preflight;
    uint32_t lifecycle_valid;cs_life_result lifecycle;uint32_t manager_valid;cs_mm_result manager;
    uint32_t reply_valid;cs_mm_reply reply;
    uint32_t guest_committed,native_committed,attempted,completed,moved,world_bytes,table_bytes;
    uint32_t segment_valid,segment,masks_valid;
    uint16_t allocated,locked,copied,live,disposed,retired;
    uint32_t execution_authorized;
    uint32_t execution_scrubbed;
} cs_image_report;
typedef struct { cs_image_state state;uint32_t generation,generation_valid,epoch,epoch_valid;
    uint32_t entry_count,segment_count,warnings,execution_authorized;cs_code_layout layout;
    uint16_t live;cs_image_segment segments[CS_CODE_SEGMENTS];
} cs_image_description;
typedef struct cs_image_service { uint32_t tag;const struct cs_image_service *self;
    cs_life_controller *lifecycle;cs_mm_manager *manager;const cs_guest_map *map;
    const cs_guest_region *descriptors;size_t region_count;cs_guest_region regions[CS_GUEST_MAX_REGIONS];
    cs_mm_config config;cs_image_profile profile;cs_image_state state;
    uint32_t last_generation,generation,epoch,reservation,staging_borrow;
    uint8_t *staging;size_t staging_capacity;uint32_t staging_bytes,offsets[CS_CODE_SEGMENTS];
    cs_code_manifest manifest;cs_code_layout layout;cs_mm_manager sealed_manager;
    uint32_t globals[6],cells[2][CS_MM_BLOCKS];
    cs_life_entry inventory[CS_LIFE_INVENTORY];size_t inventory_count;
    const cs_res_service *resources[CS_IMAGE_RESOURCES];size_t resource_count;
    const cs_m68k_session *session;cs_m68k_call_config session_config;
    cs_m68k_bus_options session_options;const cs_trap_gateway *gateway;
    cs_image_segment segments[CS_CODE_SEGMENTS];
    uint16_t allocated,locked,copied,live,disposed,retired;uint32_t world_written;
    cs_image_report first_fault;
    /* SPEC-0035 exclusive embedded execution participant, one use/generation.
     * 0 ready,1 claimed,2 running,3 clean stop,4 guest stop,5 quarantine. */
    uint32_t execution_state,execution_generation,execution_scrubbed;
#ifdef CLASSICK_IMAGE_TEST_FAULTS
    uint32_t test_after,test_call,test_index,test_prepare;
#endif
} cs_image_service;
typedef struct { cs_code_workspace parser;cs_code_inspect_request inspect;
    cs_code_layout_request check;cs_code_report report;cs_code_manifest manifest;
    cs_code_layout layout;uint32_t globals[6],cells[2][CS_MM_BLOCKS];
} cs_image_workspace;
/* Native storage starts zeroed, truthful/live/aligned/synchronized and disjoint.
 * Scratch capacity is bytes, at least sizeof(cs_image_workspace). Source is
 * borrowed only during PREPARE. Retained staging is immutable until successful
 * LOAD/CANCEL/PARTIAL RELEASE; faults retain it. No executable authority.
 * A canonical description changes only on success. Safe reports/scratch may
 * change on refusal; unsafe native-envelope refusal preserves all outputs.
 * Inventory completeness/opaque reference semantics are host preconditions,
 * not pointer discovery or authentication. Only this image's FUTURE/REFUSE,
 * known six GLOBAL views, reference-free NATIVE/PRESERVE and stable GUEST spans
 * outside both zones/world/stack are supported; other kinds refuse.
 * Low-level APIs are not automatically intercepted by the composite gate. */
/* PREPARE supplies source and staging. All subsequent requests clear those
 * four fields; the owner alone retains staging. LOAD repeats sealed B/S/H_min.
 * CANCEL/RELEASE require the captured generation/epoch. DESCRIBE does not grant
 * another lease. Reports of successful retirement retain the retired identity
 * and actual effect masks; the EMPTY description has no live identity. */
cs_image_result cs_image_init(cs_image_service *s,cs_life_controller *lifecycle,const cs_image_profile *profile);
cs_image_result cs_image_execute(cs_image_service *s,const cs_image_request *request,
    cs_image_workspace *scratch,size_t capacity,cs_image_description *out,cs_image_report *report);
cs_image_result cs_image_external_admit(const cs_image_service *s);
#ifdef CLASSICK_IMAGE_TEST_FAULTS
/* Internal conformance only: post-unit or before manager call1=allocation,
 * 2=lock,3=disposal at one-based segment index. prepare!=0 refuses after staging,
 * before publication. No callback/production injection symbol. */
cs_image_result cs_image_test_fault(cs_image_service *s,uint32_t after,uint32_t call,uint32_t index,uint32_t prepare);
#endif
#endif
