/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "code-stream-load.h"

typedef struct { uint8_t pad; cs_loaded_segment value; } loaded_alignment;

static int overlap(uintptr_t a, size_t n, uintptr_t b, size_t m)
{
    return a < b + m && b < a + n;
}

cs_cstream_result cs_code_stream_load(const cs_cstream_manifest *m, uint32_t slot,
    uint8_t *candidate, size_t capacity, cs_loaded_segment *out)
{
    uintptr_t a = (uintptr_t)m, b = (uintptr_t)candidate, c = (uintptr_t)out;
    cs_cstream_row row;
    cs_cstream_result resolved;
    cs_rfs_report report;
    uint8_t staging[CS_RFS_CHUNK];
    const cs_rf_entry *entry;
    uint32_t size, copied = 0;
    if (!m || !candidate || !out || capacity == 0 || capacity > CS_RFS_NATIVE_LIMIT ||
        sizeof *m > UINTPTR_MAX - a || capacity > UINTPTR_MAX - b || sizeof *out > UINTPTR_MAX - c ||
        c % offsetof(loaded_alignment,value) != 0 || overlap(a,sizeof *m,b,capacity) ||
        overlap(a,sizeof *m,c,sizeof *out) || overlap(b,capacity,c,sizeof *out))
        return CS_CSTREAM_ARGUMENT;
    resolved = cs_code_stream_resolve_row(m, slot, &row);
    if (resolved != CS_CSTREAM_OK) return resolved;
    entry = &m->segments[row.segment_index].resource.entry;
    size = entry->data.length - 4u;
    if (size > capacity) return CS_CSTREAM_PROFILE_LIMIT;
    while (copied < size) {
        size_t take = (size-copied > CS_RFS_CHUNK) ? CS_RFS_CHUNK : (size_t)(size-copied);
        if (cs_rfs_read(&m->source, entry, CS_RFS_PAYLOAD, copied + 4u, take,
                candidate+(size_t)copied, take, staging, sizeof staging, &report) != CS_RFS_OK) {
            size_t i;
            for (i = 0; i < (size_t)size; ++i) candidate[i] = 0;
            return CS_CSTREAM_SOURCE;
        }
        copied += (uint32_t)take;
    }
    out->size = size;
    out->entry_offset = row.resource_offset - 4u;
    return CS_CSTREAM_OK;
}
