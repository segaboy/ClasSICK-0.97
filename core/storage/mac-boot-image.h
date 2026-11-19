/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MAC_BOOT_IMAGE_H
#define CLASSICK_MAC_BOOT_IMAGE_H
#include "mac-boot-header.h"

typedef struct {
    uint8_t blocks[CS_MAC_BOOT_AREA_BYTES];
    cs_mac_boot_header header;
} cs_mac_boot_image;

/* SPEC-0130. Output and workspace are distinct live native objects and are
 * disjoint from the device and its backing. Output survives every refusal. */
cs_mac_boot_result cs_mac_boot_image_load(const cs_block_device *device,
    cs_mac_boot_workspace *workspace, cs_mac_boot_image *image);

#endif
