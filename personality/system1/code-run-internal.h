/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Private coordinator boundary. */
#ifndef CLASSICK_SYSTEM1_CODE_RUN_INTERNAL_H
#define CLASSICK_SYSTEM1_CODE_RUN_INTERNAL_H
#include "code-run.h"
typedef struct {
    cs_guest_view argument,rect,scratch;
    cs_trap_frame next;
    uint8_t bytes[8];
} cs_image_rect_scratch;
int cs_image_execution_rect_native(cs_image_service *s,const cs_image_run_request *q,
    cs_trap_frame *caller,cs_image_run_report *r,cs_qd_rect_port *port,
    cs_image_rect_scratch *scratch);
int cs_image_execution_native(cs_image_service *s,const cs_image_run_request *q,
    cs_trap_frame *caller,cs_image_run_report *r);
int cs_image_execution_retire_native(cs_image_service *s,const cs_image_run_request *q,
    cs_image_workspace *w,size_t capacity,cs_image_description *out,cs_image_report *r);
int cs_image_execution_integrity(const cs_image_service *s,const cs_image_context *c,int dirty);
#endif
