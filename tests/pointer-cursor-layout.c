/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Contract-only native ABI assertions. */
#include <stddef.h>
#include "../core/input/pointer.h"
#include "../core/graphics/cursor.h"
#define CP_PTR sizeof(void *)
_Static_assert(sizeof(cs_pointer)==20u,"pointer state size");
_Static_assert(sizeof(cs_pointer_sample)==12u,"sample size");
_Static_assert(sizeof(cs_pointer_event)==16u,"event size");
_Static_assert(offsetof(cs_pointer,valid)==16u,"pointer marker offset");
_Static_assert(sizeof(cs_cursor_state)==(CP_PTR==8u?56u:48u),"cursor state size");
_Static_assert(offsetof(cs_cursor_state,magic)==2u*CP_PTR,"cursor magic offset");
_Static_assert(offsetof(cs_cursor_state,origin_x)==2u*CP_PTR+20u,"cursor origin offset");
_Static_assert(offsetof(cs_cursor_state,save_left)==2u*CP_PTR+28u,"cursor save offset");
_Static_assert(offsetof(cs_cursor_state,phase)==2u*CP_PTR+36u,"cursor phase offset");
_Static_assert(sizeof(cs_cursor_bits)==64u,"art bits size");
_Static_assert(sizeof(cs_cursor_art)==(CP_PTR==8u?16u:8u),"art descriptor size");
_Static_assert(sizeof(cs_cursor_token)==8u,"token size");
_Static_assert(sizeof(cs_cursor_info)==28u,"query info size");
_Static_assert(sizeof(cs_surface)==(CP_PTR==8u?40u:24u),"surface size");
