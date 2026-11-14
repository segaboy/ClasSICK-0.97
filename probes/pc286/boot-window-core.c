/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Execute actual shared pointer/window semantics in the bounded 286 harness.
 * The caller establishes one near-data/stack segment; no hardware input,
 * rendering, OS startup or general far-pointer qualification is claimed. */
#include "../../arch/i286/c16.h"
#include "../../core/ui/window.h"

#if !defined(__WATCOMC__) || !defined(_M_I86)
#error This fixture requires the pinned 16-bit Open Watcom target
#endif
CS_C16_ASSERT(cs_window_boot_near_data, sizeof(void *) == 2);

static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void fail(unsigned char code) { post(code); for (;;) { } }

static void sample(cs_pointer *p, cs_window *w, int32_t dx, int32_t dy,
    uint32_t buttons, cs_pointer_event *event, cs_window_effect *effect)
{
    cs_pointer_sample in;
    int result;
    in.dx=dx; in.dy=dy; in.buttons=buttons;
    result=cs_pointer_sample_apply(p,&in,event);
    if (result!=CS_POINTER_OK && result!=CS_POINTER_CHANGED) fail(0xd1);
    result=cs_window_apply(w,event,effect);
    if (result!=CS_WINDOW_OK && result!=CS_WINDOW_CHANGED) fail(0xd2);
}

void start(void)
{
    cs_pointer pointer;
    cs_window window;
    cs_window_config config;
    cs_pointer_event event;
    cs_window_effect effect;
    unsigned char __far *out=(unsigned char __far *)0x60000000UL;
    unsigned char expected_x=112;

    config.x=80; config.y=70; config.width=224; config.height=144; config.title=18;
    if (cs_pointer_begin(&pointer,0)!=CS_POINTER_OK ||
        cs_window_begin(&window,&config)!=CS_WINDOW_OK) fail(0xd0);
    sample(&pointer,&window,-156,-91,0,&event,&effect); /* 100,80 */
    sample(&pointer,&window,0,0,1,&event,&effect);
    if (!window.dragging || effect.flags!=CS_WINDOW_DRAG_BEGAN) fail(0xd3);
    sample(&pointer,&window,32,0,1,&event,&effect);
    if (window.x!=112 || window.y!=70 || effect.flags!=CS_WINDOW_MOVED) fail(0xd4);
    sample(&pointer,&window,0,-24,0,&event,&effect);
    if (window.x!=112 || window.y!=46 || window.dragging ||
        effect.flags!=(CS_WINDOW_MOVED|CS_WINDOW_DRAG_ENDED)) fail(0xd5);
    sample(&pointer,&window,16,0,0,&event,&effect);
    if (window.x!=112 || window.y!=46) fail(0xd6);
    sample(&pointer,&window,2,54,0,&event,&effect); /* 150,110 */
    sample(&pointer,&window,0,0,1,&event,&effect);
    if (window.pattern!=1 || effect.flags!=CS_WINDOW_TOGGLED) fail(0xd7);
    sample(&pointer,&window,0,0,1,&event,&effect);
    if (window.pattern!=1 || effect.flags!=0) fail(0xd8);
    sample(&pointer,&window,0,0,0,&event,&effect);
    effect.flags=0x12345678UL;
    event.flags=0x00010000UL; event.buttons=0;
    if (cs_window_apply(&window,&event,&effect)!=CS_WINDOW_ARGUMENT ||
        effect.flags!=0x12345678UL || window.pattern!=1 || window.down) fail(0xd9);
    event.flags=0; event.buttons=0x80000000UL;
    if (cs_window_apply(&window,&event,&effect)!=CS_WINDOW_ARGUMENT ||
        effect.flags!=0x12345678UL || window.pattern!=1 || window.down) fail(0xda);
#ifdef CS_P286_BAD_EXPECTATION
    expected_x=113; /* wrong-expectation control, not an implementation mutant */
#endif
    if (window.x!=expected_x) fail(0xdb);
    out[0]='W';out[1]='2';out[2]='8';out[3]='6';
    out[4]=(unsigned char)window.x;out[5]=(unsigned char)window.y;
    out[6]=(unsigned char)window.pattern;out[7]=(unsigned char)window.dragging;
    out[8]=(unsigned char)pointer.x;out[9]=(unsigned char)pointer.y;
    out[10]=(unsigned char)window.down;out[11]=0x5a;
    post(0xa9); for (;;) { }
}
