/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_CORE_UI_WINDOW_H
#define CS_CORE_UI_WINDOW_H
#include "../input/pointer.h"
/* SPEC-0053. Caller-owned native objects; one coordinated, nonreentrant owner.
 * No drawing, physical input, guest ABI or retained pointers. */
typedef struct { int32_t x,y,width,height,title; } cs_window_config;
typedef struct {
    int32_t x,y,width,height,title,grab_x,grab_y;
    uint32_t down,dragging,pattern,valid;
} cs_window;
typedef struct { int32_t old_x,old_y,x,y; uint32_t flags; } cs_window_effect;
#define CS_WINDOW_VALID 0x57494E31u
enum { CS_WINDOW_OK=0, CS_WINDOW_CHANGED=1, CS_WINDOW_ARGUMENT=2, CS_WINDOW_STATE=3 };
enum { CS_WINDOW_MOVED=1u, CS_WINDOW_DRAG_BEGAN=2u,
       CS_WINDOW_DRAG_ENDED=4u, CS_WINDOW_TOGGLED=8u };
/* Fresh/quiescent coordinated session only. Never reads old *window. */
int cs_window_begin(cs_window *window,const cs_window_config *config);
/* Deliver each authentic accepted pointer output once, including unchanged
 * outputs. Refusal is atomic here; it cannot roll back the upstream pointer. */
int cs_window_apply(cs_window *window,const cs_pointer_event *event,cs_window_effect *out);
#endif
