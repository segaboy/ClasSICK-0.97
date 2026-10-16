/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_CODE_RUN_H
#define CLASSICK_SYSTEM1_CODE_RUN_H
#include "code-image.h"
#include "../../core/cpu/m68k-guard.h"
#include "quickdraw-rects.h"
typedef enum {CS_IMAGE_RUN_RETURNED=0,CS_IMAGE_RUN_ARGUMENT=1,CS_IMAGE_RUN_REFUSED=2,
    CS_IMAGE_RUN_CPU=3,CS_IMAGE_RUN_BUDGET=4,CS_IMAGE_RUN_QUARANTINED=5} cs_image_run_result;
typedef struct {
    cs_image_context context;
    cs_trap_gateway *gateway;
    uint32_t generation,epoch,token,budget;
} cs_image_run_request;
typedef struct {
    cs_image_run_result status;cs_m68k_result cpu;cs_guest_result memory;
    uint32_t generation,epoch,steps,entered;
    cs_trap_frame guest;
} cs_image_run_report;
/* SPEC-0035 dedicated single-root, one-entry/one-segment demo session.
 * No alternate executor or traps.
 * Only the image's sole FUTURE/REFUSE inventory entry; no session/resources.
 * Predeclared token is 8 RX bytes (four NOPs) outside heap/world/stack.
 * One run per loaded generation. Caller frame unchanged; report has stop frame.
 * Native objects/buffers live/aligned/disjoint/synchronized; no hidden references.
 * No low-level host entry, mutation, callback, patch or re-entry during a run.
 * Ordinary unsafe envelope refusal preserves report too. Stopped effects remain
 * until explicit retirement scrubs below-A5 and uses original RELEASE. */
cs_image_run_result cs_image_run(cs_image_service *image,const cs_image_run_request *request,
    cs_trap_frame *caller,cs_image_run_report *report);
/* SPEC-0036 opt-in fixed-port rectangle profile. Only canonical A8A2/A8A3;
 * all other traps and ordinary cs_image_run remain refused. Full native port
 * extent is disjoint from controls/outputs/guest backings. Borrowed for this
 * synchronous run only; its actual pixels/counts survive stop/retirement. */
cs_image_run_result cs_image_run_rectangles(cs_image_service *image,
    const cs_image_run_request *request,cs_trap_frame *caller,
    cs_image_run_report *report,cs_qd_rect_port *port);
cs_image_result cs_image_run_retire(cs_image_service *image,const cs_image_run_request *request,
    cs_image_workspace *scratch,size_t capacity,cs_image_description *out,cs_image_report *report);
#endif
