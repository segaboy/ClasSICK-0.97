/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_MEDIA_H
#define CLASSICK_PC_MEDIA_H
#include <stddef.h>
#include <stdint.h>

/* SPEC-0014 fixed boot-media profile: 512-byte blocks, 64-MiB GPT disk, one
   FAT32 EFI System Partition holding \EFI\BOOT\BOOTX64.EFI and nothing else. */
enum {
    CS_MEDIA_SECTOR=512, CS_MEDIA_DISK_SECTORS=131072, CS_MEDIA_ENTRIES=128,
    CS_MEDIA_ENTRY_SIZE=128, CS_MEDIA_ENTRY_SECTORS=32, CS_MEDIA_FIRST_USABLE=34,
    CS_MEDIA_LAST_USABLE=131038, CS_MEDIA_BACKUP_ENTRIES=131039, CS_MEDIA_BACKUP_HEADER=131071,
    CS_MEDIA_PART_FIRST=2048, CS_MEDIA_PART_SECTORS=126976,
    CS_MEDIA_RESERVED=78, CS_MEDIA_FATS=2, CS_MEDIA_FAT_SECTORS=985,
    CS_MEDIA_DATA_FIRST=2048, CS_MEDIA_CLUSTERS=124928, CS_MEDIA_FIRST_PAYLOAD_CLUSTER=5,
    CS_MEDIA_MAX_PAYLOAD_CLUSTERS=124925
};
#define CS_MEDIA_BYTES ((size_t)CS_MEDIA_SECTOR*(size_t)CS_MEDIA_DISK_SECTORS)
#define CS_MEDIA_MAX_PAYLOAD ((size_t)CS_MEDIA_SECTOR*(size_t)CS_MEDIA_MAX_PAYLOAD_CLUSTERS)
typedef enum { CS_MEDIA_OK=0, CS_MEDIA_ARGUMENT=1, CS_MEDIA_CAPACITY=2 } cs_media_result;
typedef struct {
    uint32_t payload_clusters, free_clusters, next_free;
    uint32_t header_crc, backup_crc, entries_crc;
} cs_media_layout;

/* Reflected CRC-32, polynomial 0x04C11DB7, initial/final all ones (UEFI tables/GPT). */
uint32_t cs_media_crc32(const unsigned char *data,size_t size);
/* Writes every byte of image (exactly CS_MEDIA_BYTES) from a nonempty payload that
   must not overlap it. Image and out are untouched unless CS_MEDIA_OK is returned. */
cs_media_result cs_media_build(const unsigned char *payload,size_t payload_size,
    unsigned char *image,size_t image_size,cs_media_layout *out);
#endif
