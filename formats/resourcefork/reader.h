/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_RESOURCEFORK_READER_H
#define CLASSICK_RESOURCEFORK_READER_H
#include <stddef.h>
#include <stdint.h>
typedef enum {
    CS_RF_OK=0,CS_RF_REFERENCE=1,CS_RF_NOT_FOUND=2,CS_RF_ARGUMENT=3,
    CS_RF_TRUNCATED=4,CS_RF_OUT_OF_RANGE=5,CS_RF_OVERLAP=6,CS_RF_OVERFLOW=7,
    CS_RF_NAME=8,CS_RF_DUPLICATE_TYPE=9,CS_RF_DUPLICATE_ID=10
} cs_rf_result;
typedef enum {
    CS_RF_FIELD_NONE=0,CS_RF_FIELD_HEADER=1,CS_RF_FIELD_DATA=2,CS_RF_FIELD_MAP=3,
    CS_RF_FIELD_TYPE_LIST=4,CS_RF_FIELD_NAME_LIST=5,CS_RF_FIELD_REFERENCES=6,
    CS_RF_FIELD_NAME=7,CS_RF_FIELD_PAYLOAD=8,CS_RF_FIELD_TYPE_KEY=9,CS_RF_FIELD_ID=10
} cs_rf_field;
#define CS_RF_WARN_LAYOUT 1u
#define CS_RF_WARN_MAP_RESERVED 2u
#define CS_RF_WARN_HANDLE 4u
#define CS_RF_WARN_ATTRIBUTES 8u
#define CS_RF_WARN_REFERENCE 16u
#define CS_RF_WARN_EMPTY_LIST 32u
#define CS_RF_WARN_LARGE 64u
typedef struct {uint32_t offset,length;} cs_rf_span;
typedef struct {
    const uint8_t *bytes;uint32_t tag,length;
    cs_rf_span data,map,reserved,application;
    uint32_t type_list,name_list,type_count,warnings;
} cs_rf_view;
typedef struct {
    uint32_t type;int32_t id;uint32_t reference,attributes,name_present,specification;
    cs_rf_span name,data;
} cs_rf_entry;
typedef struct {
    cs_rf_result status;cs_rf_field field;uint32_t position,warnings,types,references;
} cs_rf_report;
/* SPEC-0024: truthful immutable contiguous input and disjoint live native
 * objects. View is constructor-produced and immutable until last lookup.
 * Spans are file offsets, not native/guest addresses; no content is executed.
 * Argument refusal preserves all outputs. Other failures preserve view/entry. */
cs_rf_result cs_rf_open(const uint8_t *bytes,size_t length,cs_rf_view *view,cs_rf_report *report);
cs_rf_result cs_rf_find(const cs_rf_view *view,uint32_t type,int32_t id,
    cs_rf_entry *entry,cs_rf_report *report);
#endif
