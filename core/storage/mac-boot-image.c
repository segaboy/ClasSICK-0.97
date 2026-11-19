/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mac-boot-image.h"

cs_mac_boot_result cs_mac_boot_image_load(const cs_block_device *device,
    cs_mac_boot_workspace *workspace, cs_mac_boot_image *image)
{
    unsigned i;
    cs_mac_boot_result result;
    if (image == NULL || (const void *)image == (const void *)workspace ||
        (const void *)image == (const void *)device)
        return CS_MAC_BOOT_ARGUMENT;

    result = cs_mac_boot_header_read(device, workspace, &image->header);
    if (result != CS_MAC_BOOT_OK)
        return result;
    for (i = 0; i < sizeof image->blocks; ++i)
        image->blocks[i] = workspace->area[i];
    return CS_MAC_BOOT_OK;
}
