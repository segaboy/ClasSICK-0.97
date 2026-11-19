/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_CODE_STREAM_LOAD_H
#define CLASSICK_SYSTEM1_CODE_STREAM_LOAD_H
#include "code-stream-row.h"
typedef struct { uint32_t size, entry_offset; } cs_loaded_segment;
/* SPEC-0134. Candidate is fresh and unpublished. Read failure clears the
 * attempted body. All failures preserve the output descriptor. */
cs_cstream_result cs_code_stream_load(const cs_cstream_manifest *manifest, uint32_t slot,
    uint8_t *candidate, size_t capacity, cs_loaded_segment *out);
#endif
