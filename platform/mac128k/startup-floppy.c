/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "startup-floppy.h"

static cs_block_io_result startup_read(void *context, uint32_t lba, uint8_t *block)
{
    cs_mac_startup_floppy *reader = context;
    cs_block_io_result result;
    if (reader == NULL) return CS_BLOCK_IO_ERROR;
    /* Preflight refusals in the single-attempt provider may retain the
     * previous decode status. Only a mismatch from this read warrants retry. */
    reader->drive.error = CS_MAC_IWM_ARGUMENT;
    result = cs_mac_iwm_read_one(&reader->drive, lba, block);
    if (result == CS_BLOCK_IO_ERROR && reader->drive.error == CS_MAC_IWM_CHECKSUM) {
        ++reader->checksum_retries;
        result = cs_mac_iwm_read_one(&reader->drive, lba, block);
    }
    return result;
}

int cs_mac_startup_floppy_bind(cs_mac_startup_floppy *reader, cs_block_device *device)
{
    if (reader == NULL || !cs_mac_iwm_bind(&reader->drive, device)) return 0;
    reader->checksum_retries = 0;
    device->context = reader;
    device->read = startup_read;
    return 1;
}
