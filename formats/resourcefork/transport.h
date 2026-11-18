/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_RESOURCEFORK_TRANSPORT_H
#define CLASSICK_RESOURCEFORK_TRANSPORT_H
#include "reader.h"
#define CS_RFS_CHUNK 256u
#define CS_RFS_NATIVE_LIMIT 32767u
typedef enum {
    CS_RFS_OK=0,CS_RFS_REFERENCE=1,CS_RFS_NOT_FOUND=2,CS_RFS_ARGUMENT=3,
    CS_RFS_TRUNCATED=4,CS_RFS_OUT_OF_RANGE=5,CS_RFS_OVERLAP=6,CS_RFS_OVERFLOW=7,
    CS_RFS_NAME=8,CS_RFS_DUPLICATE_TYPE=9,CS_RFS_DUPLICATE_ID=10,
    CS_RFS_UNAVAILABLE=11,CS_RFS_IO=12,CS_RFS_PROVIDER=13,CS_RFS_CAPACITY=14
} cs_rfs_result;
typedef enum { CS_RFS_IO_OK=0,CS_RFS_IO_UNAVAILABLE=1,CS_RFS_IO_ERROR=2,
    CS_RFS_IO_PROVIDER=3 } cs_rfs_io;
/* Success fills count 1..256 bytes. Failure may change destination, never beyond
 * count. No retained pointer. Source/context/media stay immutable and stable. */
typedef cs_rfs_io (*cs_rfs_read_at)(void *context,uint32_t offset,
    uint16_t count,uint8_t *destination);
typedef struct {uint32_t length;cs_rfs_read_at read;void *context;} cs_rfs_source;
typedef struct {uint32_t offset;uint16_t count;uint8_t bytes[CS_RFS_CHUNK];} cs_rfs_workspace;
typedef struct {
    cs_rfs_source source;uint32_t tag;
    cs_rf_span data,map,reserved,application;
    uint32_t type_list,name_list,type_count,warnings;
} cs_rfs_view;
typedef struct {
    cs_rfs_result status;cs_rf_field field;uint32_t position,warnings,types,references;
} cs_rfs_report;
typedef enum {CS_RFS_PAYLOAD=0,CS_RFS_RESOURCE_NAME=1} cs_rfs_part;
/* SPEC-0116. All controls, provider backing, scratch and destinations are live,
 * disjoint native objects in individual segments. No arbitrary alias/pointer
 * validity detection. Source and constructor-produced view/entry stay stable.
 * Argument failures preserve all outputs including report. Other failures can
 * replace report/scratch/provider state, but preserve view/entry/output bytes.
 * Chunk cache may read adjacent bytes within source.length, not beyond it.
 */
cs_rfs_result cs_rfs_open(const cs_rfs_source *source,cs_rfs_workspace *scratch,
    cs_rfs_view *view,cs_rfs_report *report);
cs_rfs_result cs_rfs_find(const cs_rfs_view *view,cs_rfs_workspace *scratch,
    uint32_t type,int32_t id,cs_rf_entry *entry,cs_rfs_report *report);
cs_rfs_result cs_rfs_read(const cs_rfs_view *view,const cs_rf_entry *entry,
    cs_rfs_part part,uint32_t offset,size_t count,uint8_t *output,
    size_t output_capacity,uint8_t *staging,size_t staging_capacity,
    cs_rfs_report *report);
#endif
