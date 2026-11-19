/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_CODE_STREAM_ROW_H
#define CLASSICK_SYSTEM1_CODE_STREAM_ROW_H
#include "code-stream.h"

typedef struct {
    uint32_t segment_index;
    uint32_t resource_offset;
} cs_cstream_row;

/* SPEC-0132. Ordinary inactive context; retain the immutable source owner and
 * published manifest until return. No target instruction is read. */
cs_cstream_result cs_code_stream_resolve_row(const cs_cstream_manifest *manifest,
    uint32_t slot, cs_cstream_row *out);
#endif
