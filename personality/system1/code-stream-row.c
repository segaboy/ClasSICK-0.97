/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "code-stream-row.h"

typedef struct { uint8_t pad; cs_cstream_manifest value; } manifest_alignment;
typedef struct { uint8_t pad; cs_cstream_row value; } row_alignment;

static uint32_t word(const uint8_t *p)
{
    return ((uint32_t)p[0] << 8) | p[1];
}

cs_cstream_result cs_code_stream_resolve_row(const cs_cstream_manifest *m,
    uint32_t slot, cs_cstream_row *out)
{
    uintptr_t manifest_at = (uintptr_t)m, output_at = (uintptr_t)out;
    const cs_cstream_segment *segment;
    uint8_t bytes[8], scratch[8];
    cs_rfs_report detail;
    uint32_t index, offset, length;

    if (m == NULL || out == NULL ||
        manifest_at % offsetof(manifest_alignment, value) != 0 ||
        output_at % offsetof(row_alignment, value) != 0 ||
        sizeof *m > UINTPTR_MAX - manifest_at || sizeof *out > UINTPTR_MAX - output_at ||
        (manifest_at < output_at + sizeof *out && output_at < manifest_at + sizeof *m))
        return CS_CSTREAM_ARGUMENT;
    if (m->tag != UINT32_C(0x43534d32) || m->segment_count > CS_CSTREAM_SEGMENTS ||
        m->code0.entry.data.length < 16u || m->table_length > m->code0.entry.data.length - 16u ||
        (m->table_length & 7u) || m->entry_count != (m->table_length >> 3) ||
        (m->routine_base != CS_CSTREAM_RESOURCE_START && m->routine_base != CS_CSTREAM_AFTER_HEADER) ||
        slot >= m->entry_count)
        return CS_CSTREAM_ARGUMENT;

    for (index = 0; index < m->segment_count; ++index) {
        segment = &m->segments[index];
        if (slot >= segment->first_entry && slot - segment->first_entry < segment->entry_count)
            break;
    }
    if (index == m->segment_count)
        return CS_CSTREAM_SEGMENT_HEADER;
    if (cs_rfs_read(&m->source, &m->code0.entry, CS_RFS_PAYLOAD, 16u + (slot << 3),
            sizeof bytes, bytes, sizeof bytes, scratch, sizeof scratch, &detail) != CS_RFS_OK)
        return CS_CSTREAM_SOURCE;
    if (word(bytes + 2) != 0x3f3cu || (int32_t)word(bytes + 4) != segment->resource.entry.id ||
        word(bytes + 6) != 0xa9f0u)
        return CS_CSTREAM_ENTRY_FORM;
    offset = word(bytes);
    if (m->routine_base == CS_CSTREAM_AFTER_HEADER)
        offset += 4u;
    length = segment->resource.entry.data.length;
    if ((offset & 1u) || offset < 4u || offset > length || length - offset < 2u)
        return CS_CSTREAM_ROUTINE_RANGE;
    out->segment_index = index;
    out->resource_offset = offset;
    return CS_CSTREAM_OK;
}
