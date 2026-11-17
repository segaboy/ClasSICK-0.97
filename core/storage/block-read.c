/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "block-read.h"

cs_block_result cs_block_read(const cs_block_device *device, uint32_t lba,
    uint32_t count, uint8_t *output, size_t output_capacity,
    uint8_t *staging, size_t staging_capacity)
{
    uint32_t block;
    size_t bytes, offset;
    if (device == NULL || device->read == NULL || device->blocks == 0 ||
        output == NULL || staging == NULL || output == staging || count == 0 ||
        output_capacity > CS_BLOCK_NATIVE_LIMIT ||
        staging_capacity > CS_BLOCK_NATIVE_LIMIT) return CS_BLOCK_ARGUMENT;
    if (count > CS_BLOCK_MAX_COUNT) return CS_BLOCK_RANGE;
    if (lba >= device->blocks || count > device->blocks - lba)
        return CS_BLOCK_RANGE;
    /* The admitted count makes this product <=32256 even with 16-bit size_t.
     * Never multiply or add the untrusted LBA to obtain a native address. */
    bytes = (size_t)(count << 9);
    if (output_capacity < bytes || staging_capacity < bytes)
        return CS_BLOCK_CAPACITY;
    offset = 0;
    for (block = 0; block < count; ++block) {
        cs_block_io_result io = device->read(device->context, lba + block,
            staging + offset);
        if (io != CS_BLOCK_IO_OK) {
            if (io == CS_BLOCK_IO_UNAVAILABLE) return CS_BLOCK_UNAVAILABLE;
            if (io == CS_BLOCK_IO_ERROR) return CS_BLOCK_IO;
            return CS_BLOCK_PROVIDER;
        }
        offset += CS_BLOCK_BYTES;
    }
    for (offset = 0; offset < bytes; ++offset) output[offset] = staging[offset];
    return CS_BLOCK_OK;
}
