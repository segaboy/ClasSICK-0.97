/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_MONO_PRESENT_H
#define CLASSICK_PC_MONO_PRESENT_H
#include "framebuffer.h"

/* SPEC-0060: stateless 512x342 Mono1 RAM -> centered native framebuffer.
 * Caller owns initial/full publication sequencing and all live mappings. */
enum { CS_MONO_OK=0, CS_MONO_ARGUMENT=1, CS_MONO_SOURCE=2,
       CS_MONO_TARGET=3, CS_MONO_CAPACITY=4, CS_MONO_ALIAS=5,
       CS_MONO_PRESENT=6 };
enum { CS_MONO_VALIDATED=1, CS_MONO_COMPLETE=2, CS_MONO_FAILED=3 };
typedef struct {
    uint32_t phase,rows,required;
    int32_t cause;
} cs_pc_mono_report;

/* initial is 0/1. Canvas: MONO1_MSB,512x342,stride64,>=21888 live bytes.
 * Target >=512x342,subject to cs_fb_init limits. Row capacity >=4*target.width
 * in BOTH modes;only the used row prefix is written. initial1 publishes every
 * physical row with white margins;initial0 publishes the512x342 canvas only.
 * Preconditions: exclusive synchronous use,truthful live aligned objects and
 * disjoint writable spans (target.size,row_capacity,report) vs each other and
 * readonly target/canvas descriptors and canvas.storage_size. Readonly inputs
 * may share. Numeric spans cannot prove memory accessibility or device mapping.
 * Preflight order: fixed NULL/wrap/initial -> SOURCE -> TARGET -> CAPACITY ->
 * ALIAS. No caller writes/pixel reads before all preflight succeeds. Refusal
 * leaves all caller bytes unchanged. Source and descriptors stay unchanged.
 * Then report VALIDATED/rows0/required(H or342)/cause0;each complete present
 * row increments rows. Dependency refusal sets FAILED/cause to raw CS_FB_* and
 * returns PRESENT;earlier rows/workspace effects remain. Success COMPLETE.
 * Hardware exceptions are not C errors or recoverable publication failures.
 * No allocation,retained borrow,UEFI types,framebuffer reads or direct device
 * writes:reuse cs_fb_init,cs_surface_init and cs_fb_present unchanged. */
int cs_pc_mono_present(const cs_fb_target *target,const cs_surface *canvas,
    uint8_t *row,size_t row_capacity,uint32_t initial,cs_pc_mono_report *out);
/* Dirty-rect variant. count 0..3; rects may be NULL only when count is 0.
 * Logical half-open noninverted rects (else ARGUMENT), clipped to 512x342
 * before subtraction; empty ones skipped; overlaps/repeats published in
 * caller order. Preflight as cs_pc_mono_present plus rect span/order in
 * ARGUMENT and the rect array as a readonly ALIAS span. Report VALIDATED,
 * rows0, required=sum of clipped heights. Each rect row converts exactly its
 * width to RGBA and presents at centered origin+left/top. Count 0 validates
 * and reports COMPLETE rows0 required0. Failure semantics as above. */
int cs_pc_mono_present_rects(const cs_fb_target *target,const cs_surface *canvas,
    uint8_t *row,size_t row_capacity,const cs_rect *rects,size_t count,
    cs_pc_mono_report *out);
#endif
