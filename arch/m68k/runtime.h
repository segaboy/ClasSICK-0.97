/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_M68K_RUNTIME_H
#define CLASSICK_M68K_RUNTIME_H
#include <stdint.h>
#include <stddef.h>
uint32_t __mulsi3(uint32_t a, uint32_t b);
uint32_t __udivsi3(uint32_t a, uint32_t b);
uint32_t __umodsi3(uint32_t a, uint32_t b);
int32_t __divsi3(int32_t a, int32_t b);
void *memcpy(void *destination, const void *source, size_t count);
#endif
