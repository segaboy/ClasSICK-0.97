/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_CODE_STREAM_H
#define CLASSICK_SYSTEM1_CODE_STREAM_H
#include "../../formats/resourcefork/transport.h"
#define CS_CSTREAM_SEGMENTS 64u
#define CS_CSTREAM_CHUNK 512u
#define CS_CSTREAM_TYPE UINT32_C(0x434f4445)
typedef enum {
    CS_CSTREAM_OK=0,CS_CSTREAM_ARGUMENT=1,CS_CSTREAM_CONTEXT=2,
    CS_CSTREAM_PROFILE_LIMIT=3,CS_CSTREAM_SOURCE=4,CS_CSTREAM_NOT_FOUND=5,
    CS_CSTREAM_AMBIGUOUS=6,CS_CSTREAM_REFERENCE=7,CS_CSTREAM_STORAGE=8,
    CS_CSTREAM_CODE0_FORMAT=9,CS_CSTREAM_ENTRY_FORM=10,
    CS_CSTREAM_SEGMENT_HEADER=11,CS_CSTREAM_ROUTINE_RANGE=12
} cs_cstream_result;
typedef enum {
    CS_CSTREAM_FIELD_NONE=0,CS_CSTREAM_FIELD_ABOVE=1,CS_CSTREAM_FIELD_BELOW=2,
    CS_CSTREAM_FIELD_TABLE_LENGTH=3,CS_CSTREAM_FIELD_TABLE_OFFSET=4,
    CS_CSTREAM_FIELD_ENTRY=5,CS_CSTREAM_FIELD_SEGMENT=6,CS_CSTREAM_FIELD_ROUTINE=7
} cs_cstream_field;
enum {CS_CSTREAM_UNKNOWN=0,CS_CSTREAM_ORDINARY=1,CS_CSTREAM_INTERRUPT=2};
typedef enum {CS_CSTREAM_RESOURCE_START=0, CS_CSTREAM_AFTER_HEADER=1} cs_cstream_base;
typedef enum {
    CS_CSTREAM_RANGE_NONE=0, CS_CSTREAM_RANGE_ODD=1, CS_CSTREAM_RANGE_HEADER=2,
    CS_CSTREAM_RANGE_BEYOND=3, CS_CSTREAM_RANGE_SHORT_TAIL=4
} cs_cstream_range_reason;
enum {CS_CSTREAM_JT_OFFSET=1,CS_CSTREAM_WORLD_SLACK=2,CS_CSTREAM_TRAILING=4};
typedef struct {cs_rf_entry entry;uint32_t reader_warnings;} cs_cstream_resource;
typedef struct {
    cs_cstream_resource resource;uint32_t first_entry,entry_count;
} cs_cstream_segment;
typedef struct {
    uint32_t tag,maximum_source,maximum_entries,maximum_segments;
    uint32_t above_a5,below_a5,table_length,table_offset,entry_count,segment_count;
    uint32_t warnings,reader_warnings,resource_start_literal;
    cs_cstream_base routine_base;
    uint32_t historical_offset_parity_unverified,execution_authorized;
    cs_rfs_view source;
    cs_cstream_resource code0;
    cs_cstream_segment segments[CS_CSTREAM_SEGMENTS];
} cs_cstream_manifest;
typedef struct {
    const cs_rfs_view *source;uint32_t maximum_source,maximum_entries;
    unsigned maximum_segments,context;
    unsigned active_root,active_callback,active_patch,active_manager;
    cs_cstream_base routine_base;
} cs_cstream_request;
typedef struct {
    cs_cstream_result cause;cs_cstream_field field;
    uint32_t key_valid;int32_t id;
    uint32_t slot_valid,slot,position_valid,position;
    uint32_t reader_valid;cs_rfs_report reader;
    uint32_t entries_checked,segments_seen,execution_authorized;
    cs_cstream_range_reason range_reason;
} cs_cstream_report;
typedef struct {
    cs_cstream_manifest candidate;
    uint32_t first[CS_CSTREAM_SEGMENTS],last[CS_CSTREAM_SEGMENTS],seen[CS_CSTREAM_SEGMENTS];
    cs_rfs_workspace resource;
    uint8_t bytes[CS_CSTREAM_CHUNK],staging[CS_CSTREAM_CHUNK];
} cs_cstream_workspace;
/* SPEC-0127: data-only streamed metadata, distinct from the old64-entry
 * cs_code_manifest. No routine bodies, guest state, allocation or execution.
 * Source is constructor-produced and immutable. All controls, source contexts,
 * media and native spans are truthful/live/disjoint, individually in native
 * segments, exclusively used with no active runtime/manager. The checked
 * native envelope covers stated controls/workspace, not hidden provider backing.
 * Failure preserves every out byte; unsafe-envelope refusal also preserves
 * workspace/report. Other failures can change scratch/report/provider state.
 * Manifest owns scalars and resource spans; its copied view borrows the ORIGINAL
 * source callback/context/media lifetime, never this construction workspace.
 * The caller must retain that source owner for future bounded runtime copies.
 * Manifest is neither authenticated ownership nor a reservation/entry authority;
 * old code-image/layout/run APIs must not consume it. */
cs_cstream_result cs_code_stream_inspect(const cs_cstream_request *request,
    cs_cstream_workspace *workspace,size_t workspace_bytes,
    cs_cstream_manifest *out,cs_cstream_report *report);
#endif
