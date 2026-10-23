/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* ABI probe, plain C90, no headers. Each typedef fails compilation if its
 * condition is false. The const objects show model-dependent sizes in the
 * object listing. Nothing here runs; this is compile-only evidence. */
typedef char abi_char8[(((unsigned char)256UL) == 0) ? 1 : -1];
typedef char abi_short16[(sizeof(short) == 2) ? 1 : -1];
typedef char abi_int16[(sizeof(int) == 2) ? 1 : -1];
typedef char abi_long32[(sizeof(long) == 4) ? 1 : -1];
typedef char abi_uint_max[((unsigned)-1 == 0xFFFFu) ? 1 : -1];
typedef char abi_ulong_max[((unsigned long)-1 == 0xFFFFFFFFUL) ? 1 : -1];
typedef char abi_near2[(sizeof(char __near *) == 2) ? 1 : -1];
typedef char abi_far4[(sizeof(char __far *) == 4) ? 1 : -1];
typedef char abi_size16[(sizeof(sizeof(int)) == 2) ? 1 : -1];

/* Default pointer sizes depend on the memory model; record them, don't assume them. */
const unsigned abi_data_ptr = sizeof(char *);
const unsigned abi_code_ptr = sizeof(void (*)(void));
const unsigned abi_ptrdiff = sizeof((char *)0 - (char *)0);

/* Shapes that mirror the frozen traps.h records, without including it. */
struct abi_frame { unsigned long d[8], a[8], pc; unsigned short sr; };
struct abi_config { unsigned long v[6]; };
struct abi_gateway { unsigned long magic; struct abi_config c;
    unsigned long entries[512]; };
const unsigned abi_frame_size = sizeof(struct abi_frame);
const unsigned abi_gateway_size = sizeof(struct abi_gateway);
typedef char abi_gateway_fits[(sizeof(struct abi_gateway) < 0xFFF0u) ? 1 : -1];

/* Near data object, then a far data object, for segment placement in the listing. */
unsigned char __near abi_near_byte = 1;
unsigned char __far abi_far_block[256];
