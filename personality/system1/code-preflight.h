/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original host-only SPEC-0032 protocol. */
#ifndef CLASSICK_SYSTEM1_CODE_PREFLIGHT_H
#define CLASSICK_SYSTEM1_CODE_PREFLIGHT_H
#include "../../core/memory/lifecycle.h"
#include "../../formats/resourcefork/reader.h"
#define CS_CODE_ENTRIES 64u
#define CS_CODE_SEGMENTS 16u
#define CS_CODE_TYPE UINT32_C(0x434F4445)
#define CS_CODE_NONCANONICAL_JT_OFFSET 1u
#define CS_CODE_WORLD_SLACK 2u
#define CS_CODE_TRAILING_BYTES 4u
typedef enum { CS_CODE_OK=0,CS_CODE_ARGUMENT=1,CS_CODE_CONTEXT=2,CS_CODE_STATE=3,
    CS_CODE_PROFILE_LIMIT=4,CS_CODE_SOURCE=5,CS_CODE_NOT_FOUND=6,CS_CODE_AMBIGUOUS=7,
    CS_CODE_UNRESOLVED_REFERENCE=8,CS_CODE_UNSUPPORTED_STORAGE=9,CS_CODE_CODE0_FORMAT=10,
    CS_CODE_ENTRY_FORM=11,CS_CODE_SEGMENT_HEADER=12,CS_CODE_ROUTINE_RANGE=13,
    CS_CODE_LAYOUT=14,CS_CODE_LAYOUT_UNSUPPORTED=15 } cs_code_result;
typedef enum { CS_CODE_INSPECT=1,CS_CODE_CHECK_LAYOUT=2 } cs_code_operation;
typedef enum { CS_CODE_FIELD_NONE=0,CS_CODE_FIELD_ABOVE=1,CS_CODE_FIELD_BELOW=2,
    CS_CODE_FIELD_TABLE_LENGTH=3,CS_CODE_FIELD_TABLE_OFFSET=4,CS_CODE_FIELD_ENTRY=5,
    CS_CODE_FIELD_SEGMENT=6,CS_CODE_FIELD_ROUTINE=7,CS_CODE_FIELD_LAYOUT=8 } cs_code_field;
typedef enum { CS_CODE_RESOURCE_START_LITERAL=1 } cs_code_offset_policy;
typedef struct { cs_life_context context;
    uint32_t active_root,active_callback,active_patch,active_manager;
    const cs_guest_map *map;
    /* Supply all relevant live runtime controls. May be absent for standalone
     * inspection without a manager; mandatory for layout. */
    const cs_life_controller *lifecycle;
    const cs_m68k_session *session;
} cs_code_context;
typedef struct { cs_rf_entry entry;uint32_t reader_warnings; } cs_code_resource;
typedef struct { uint32_t routine_offset,segment_index; } cs_code_entry;
typedef struct { cs_code_resource resource;uint32_t first_entry,entry_count; } cs_code_segment;
typedef struct { uint32_t tag,source_length,maximum_source;
    uint32_t above_a5,below_a5,table_length,table_offset,entry_count,segment_count,warnings;
    cs_code_offset_policy offset_policy;
    uint32_t historical_offset_parity_unverified,execution_authorized;
    cs_code_resource code0;
    cs_code_entry entries[CS_CODE_ENTRIES];cs_code_segment segments[CS_CODE_SEGMENTS];
} cs_code_manifest;
typedef struct { uint32_t tag,b,s,h_min,a5,globals_begin,stack_top,stack_base;
    uint32_t table_begin,table_end,candidate_appl_limit,heap_begin,heap_epoch,epoch_observed,warnings;
    uint32_t source_revalidated_for_layout,execution_authorized;
} cs_code_layout;
typedef struct { cs_code_context context;const uint8_t *source;size_t length;uint32_t maximum_source; } cs_code_inspect_request;
typedef struct { cs_code_context context;const cs_code_manifest *manifest;uint32_t b,s,h_min; } cs_code_layout_request;
typedef struct { cs_code_result cause;cs_code_operation operation;cs_code_field field;
    uint32_t key_valid;int32_t id;uint32_t slot_valid,slot,position_valid,position;
    uint32_t reader_valid;cs_rf_report reader;uint32_t lifecycle_valid;cs_life_result lifecycle;
    uint32_t execution_authorized;
} cs_code_report;
typedef struct { cs_code_manifest manifest;cs_code_layout layout;cs_rf_view view;
    cs_rf_entry entry;cs_rf_report reader;
} cs_code_workspace;
/* All native objects/buffers are truthful/live, synchronized and disjoint from
 * each other, supplied maps/backings and protected runtime controls. Scratch
 * capacity is in bytes, at least sizeof(cs_code_workspace); no host allocation.
 * Report and scratch are untouched on unsafe native-envelope refusal; other
 * failures may change them but always preserve the canonical output and runtime.
 * Manifest is constructor-produced immutable caller-owned scalar protocol data,
 * not authenticated storage, a source lease, a reservation or entry authority. */
cs_code_result cs_code_inspect(const cs_code_inspect_request *q,cs_code_workspace *scratch,
    size_t capacity,cs_code_manifest *out,cs_code_report *report);
cs_code_result cs_code_check_layout(const cs_code_layout_request *q,cs_code_workspace *scratch,
    size_t capacity,cs_code_layout *out,cs_code_report *report);
#endif
