/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0036 fixture. */
#ifndef CLASSICK_DRAWING_ROM_APP_H
#define CLASSICK_DRAWING_ROM_APP_H
#include "../loaded-rom-app/probe.h"
#include "../loaded-rom-app/scene.h"
typedef struct {cs_loaded_app_workspace app;cs_qd_rect_port port;} cs_drawing_app_workspace;
/* Original40-byte program, wrapped in an original resource fork and loaded
 * through the existing original owner. Source borrowed only for PREPARE. */
int cs_drawing_app_prepare(cs_drawing_app_workspace *w,cs_trap_gateway *gateway,
    const uint8_t *program,size_t length);
int cs_drawing_app_probe(cs_drawing_app_workspace *w,cs_trap_gateway *gateway,
    cs_loaded_app_result *result);
int cs_drawing_app_scene(const cs_surface *screen,const cs_loaded_app_result *result,
    const cs_qd_rect_port *port,uint32_t presses);
extern const uint8_t cs_drawing_app_program[40];
#endif
