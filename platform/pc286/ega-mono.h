/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_EGA_MONO_H
#define CLASSICK_PC286_EGA_MONO_H
#include <stdint.h>
#if defined(__WATCOMC__) && defined(_M_I86)
#define CS_PC286_FAR __far
#else
#define CS_PC286_FAR
#endif
/* Caller owns a 21888-byte black-is-one canvas and a writable 28000-byte
 * white-is-one EGA plane. A rectangle is clipped then rounded out to bytes.
 * Both buffers must be valid, nonoverlapping, and stable during the call. */
int cs_pc286_ega_publish(const uint8_t *canvas, uint8_t CS_PC286_FAR *plane,
                         int left, int top, int right, int bottom);
#endif
