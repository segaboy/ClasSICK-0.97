/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_ACPI_H
#define CLASSICK_PC_ACPI_H
#include <stddef.h>
#include <stdint.h>

/* SPEC-0010 bounded ACPI 2.0+ RSDP/XSDT/FADT walk for the PM timer only. */
typedef enum { CS_ACPI_OK=0, CS_ACPI_ARGUMENT=1, CS_ACPI_ACCESS=2, CS_ACPI_SIGNATURE=3,
    CS_ACPI_CHECKSUM=4, CS_ACPI_FORMAT=5, CS_ACPI_NOT_FOUND=6, CS_ACPI_DUPLICATE=7,
    CS_ACPI_UNSUPPORTED=8 } cs_acpi_result;
enum { CS_ACPI_SOURCE_LEGACY=1, CS_ACPI_SOURCE_EXTENDED=2,
    CS_ACPI_MAX_ENTRIES=256, CS_ACPI_MAX_TABLE=65536 };
/* Copies length bytes at a physical address into out; nonzero means success. */
typedef int (*cs_acpi_reader)(void *context,uint64_t physical,unsigned char *out,size_t length);
typedef struct {
    uint64_t fadt;
    uint16_t port;
    uint8_t bits,source;
} cs_acpi_pm_timer;

/* Output written only on success. Reads at most 36+8*256 XSDT bytes and 64 KiB per table. */
cs_acpi_result cs_acpi_find_pm_timer(cs_acpi_reader read,void *context,uint64_t rsdp,
    cs_acpi_pm_timer *out);
/* SPEC-0012: validate FADT revision/checksum before IAPC_BOOT_ARCH bit 1. */
cs_acpi_result cs_acpi_8042(cs_acpi_reader read,void *context,uint64_t fadt,uint32_t *out);
#endif
