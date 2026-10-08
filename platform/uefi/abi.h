/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_UEFI_ABI_H
#define CLASSICK_UEFI_ABI_H
#include <stddef.h>
#include <stdint.h>
#if UINTPTR_MAX != UINT64_MAX || (!defined(__x86_64__) && !defined(_M_X64))
#error This UEFI ABI profile requires x64 native pointer widths.
#endif
#if defined(__clang__) || defined(__GNUC__)
#define CS_EFIAPI __attribute__((ms_abi))
#else
#define CS_EFIAPI
#endif
#define CS_EFI_ERROR(n) ((UINT64_C(1)<<63)|(uint64_t)(n))
typedef uint64_t cs_efi_status;
typedef struct { uint32_t a; uint16_t b,c; unsigned char d[8]; } cs_efi_guid;
typedef struct { uint64_t signature; uint32_t revision,size,crc,reserved; } cs_efi_header;
typedef cs_efi_status (CS_EFIAPI *cs_efi_allocate)(uint32_t,uint32_t,size_t,uint64_t *);
typedef cs_efi_status (CS_EFIAPI *cs_efi_free)(uint64_t,size_t);
typedef cs_efi_status (CS_EFIAPI *cs_efi_map)(size_t *,void *,size_t *,size_t *,uint32_t *);
typedef cs_efi_status (CS_EFIAPI *cs_efi_exit)(void *,size_t);
typedef cs_efi_status (CS_EFIAPI *cs_efi_open)(void *,const cs_efi_guid *,void **,void *,void *,uint32_t);
typedef cs_efi_status (CS_EFIAPI *cs_efi_watchdog)(size_t,uint64_t,size_t,const uint16_t *);
typedef cs_efi_status (CS_EFIAPI *cs_efi_locate)(const cs_efi_guid *,void *,void **);
typedef struct {
    cs_efi_header header; uint64_t opaque0[2];
    cs_efi_allocate allocate; cs_efi_free free_pages; cs_efi_map get_map;
    uint64_t opaque1[21]; cs_efi_exit exit_boot; uint64_t opaque2[2];
    cs_efi_watchdog watchdog; uint64_t opaque3[2]; cs_efi_open open_protocol;
    uint64_t opaque4[4]; cs_efi_locate locate_protocol; uint64_t opaque5[6];
} cs_efi_boot;
typedef struct {
    cs_efi_header header; void *vendor; uint32_t firmware_revision;
    void *console_in_handle,*console_in,*console_out_handle,*console_out;
    void *error_handle,*error_out,*runtime; cs_efi_boot *boot;
    size_t configuration_count; void *configuration;
} cs_efi_system;
typedef struct {
    uint32_t revision; void *parent; cs_efi_system *system;
    void *device,*file_path,*reserved; uint32_t options_size; void *options;
    void *base; uint64_t size; uint32_t code_type,data_type; uint64_t unload;
} cs_efi_image;
typedef struct { uint32_t version,width,height,format,masks[4],pitch; } cs_efi_gop_info;
typedef struct {
    uint32_t max_mode,mode; cs_efi_gop_info *info; size_t info_size;
    uint64_t framebuffer; size_t framebuffer_size;
} cs_efi_gop_mode;
typedef struct { uint64_t query,set,blt; cs_efi_gop_mode *mode; } cs_efi_gop;
_Static_assert(sizeof(cs_efi_guid)==16 && sizeof(cs_efi_header)==24,"UEFI guid/header layout");
_Static_assert(sizeof(cs_efi_system)==120 && offsetof(cs_efi_system,boot)==96,"UEFI system layout");
_Static_assert(sizeof(cs_efi_boot)==376 && offsetof(cs_efi_boot,allocate)==40,"UEFI allocation layout");
_Static_assert(offsetof(cs_efi_boot,get_map)==56 && offsetof(cs_efi_boot,exit_boot)==232,"UEFI exit/map layout");
_Static_assert(offsetof(cs_efi_boot,open_protocol)==280 && offsetof(cs_efi_boot,watchdog)==256,"UEFI open/watchdog layout");
_Static_assert(offsetof(cs_efi_boot,locate_protocol)==320,"UEFI locate layout");
_Static_assert(sizeof(cs_efi_image)==96 && offsetof(cs_efi_image,base)==64,"UEFI image layout");
_Static_assert(sizeof(cs_efi_gop_info)==36 && offsetof(cs_efi_gop_info,pitch)==32,"UEFI pixel layout");
_Static_assert(sizeof(cs_efi_gop_mode)==40 && offsetof(cs_efi_gop_mode,framebuffer)==24,"UEFI GOP mode layout");
_Static_assert(sizeof(cs_efi_gop)==32 && offsetof(cs_efi_gop,mode)==24,"UEFI GOP layout");
#endif
