/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Compile-only ABI/constant-folding controls; never a target entry point. */
#ifndef CS_EXPECT_INT
#define CS_EXPECT_INT 2
#endif
#ifndef CS_EXPECT_FAR
#define CS_EXPECT_FAR 4
#endif
typedef char cs_int_width[(sizeof(unsigned int)==CS_EXPECT_INT)?1:-1];
typedef char cs_long_width[(sizeof(unsigned long)==4)?1:-1];
typedef char cs_char_width[(((unsigned char)256UL)==0)?1:-1];
typedef char cs_near_width[(sizeof(void __near *)==2)?1:-1];
typedef char cs_far_width[(sizeof(void __far *)==CS_EXPECT_FAR)?1:-1];
typedef char cs_old_mask_misses[((0x10001UL & ~7u)==0)?1:-1];
typedef char cs_wide_mask_catches[((0x10001UL & ~7UL)!=0)?1:-1];
const unsigned char cs_abi_values[] = {
    'C','S','1','6',sizeof(int),sizeof(short),sizeof(long),
    sizeof(void *),sizeof(void (__far *)(void)),sizeof(void (__near *)(void)),
    sizeof(void __far *),sizeof(void __near *)
};
unsigned cs_old_mask(void) { return (0x10001UL & ~7u)!=0; }
unsigned cs_wide_mask(void) { return (0x10001UL & ~7UL)!=0; }
