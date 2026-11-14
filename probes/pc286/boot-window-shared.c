/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Experimental C16 keyboard binding to the actual shared window owner and
 * compositor. Compiler runtime modules remain unreviewed for distribution. */
#include "../../arch/i286/c16.h"
#include "../../apps/window-scene/input.h"
#include "../../platform/pc286/ega-mono.h"
#include "../../platform/pc286/keyboard-pointer.h"
#include "../../platform/pc286/keyboard-set2.h"

#if !defined(__WATCOMC__) || !defined(_M_I86)
#error Requires the pinned 16-bit Open Watcom target
#endif
CS_C16_ASSERT(cs_shared_window_near, sizeof(void *) == 2);
static unsigned char canvas[21888],saved[48];
static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void video_mode(void);
#pragma aux video_mode = "mov ax,0010h" "int 10h" modify [ax bx cx dx si di];
static unsigned short read_key(void);
#pragma aux read_key = "xor ah,ah" "int 16h" value [ax] modify [ax bx cx dx si di];
#ifdef CS_P286_RAW_KEYBOARD
static void mask_keyboard_irq(void);
#pragma aux mask_keyboard_irq = "pushf" "cli" "in al,21h" "or al,02h" "out 21h,al" "popf" modify [ax];
#ifdef CS_P286_IRQ_TEST_UNMASK
static void test_unmask_keyboard_irq(void);
#pragma aux test_unmask_keyboard_irq = "pushf" "cli" "in al,21h" "and al,0fdh" "out 21h,al" "popf" modify [ax];
#endif
static unsigned char key_status(void);
#pragma aux key_status = "in al,64h" value [al] modify exact [];
static unsigned char key_byte(void);
#pragma aux key_byte = "in al,60h" value [al] modify exact [];
static unsigned char irq_mask(void);
#pragma aux irq_mask = "in al,21h" value [al] modify exact [];
static unsigned short flags_word(void);
#pragma aux flags_word = "pushf" "pop ax" value [ax] modify exact [];
#endif
static void fail(unsigned char value) { post(value); for (;;) { } }

#ifdef CS_P286_RAW_KEYBOARD
static void publish_irq(unsigned before_flags,unsigned after_flags,unsigned before_mask,unsigned after_mask)
{
    unsigned char __far *r=(unsigned char __far *)0x60000011UL;
    r[0]=(unsigned char)before_flags;r[1]=(unsigned char)(before_flags>>8);
    r[2]=(unsigned char)after_flags;r[3]=(unsigned char)(after_flags>>8);
    r[4]=(unsigned char)before_mask;r[5]=(unsigned char)after_mask;
}
#endif

static void white_display(void)
{
    unsigned i;unsigned char __far *video=(unsigned char __far *)0xa0000000UL;
    for(i=0;i<28000u;++i)video[i]=0xffu;
}

static void present(int left,int top,int right,int bottom)
{
    unsigned char __far *video=(unsigned char __far *)0xa0000000UL;
    if(!cs_pc286_ega_publish(canvas,video,left,top,right,bottom))fail(0xe5);
}

static void publish(const cs_window_input *o,unsigned count,unsigned button)
{
    unsigned char __far *r=(unsigned char __far *)0x60000000UL;
    r[0]='S';r[1]='2';r[2]='8';r[3]='6';r[4]=(unsigned char)count;r[5]=(unsigned char)button;
    r[6]=(unsigned char)o->pointer.x;r[7]=(unsigned char)(o->pointer.x>>8);
    r[8]=(unsigned char)o->pointer.y;r[9]=(unsigned char)(o->pointer.y>>8);
    r[10]=(unsigned char)o->window.x;r[11]=(unsigned char)(o->window.x>>8);
    r[12]=(unsigned char)o->window.y;r[13]=(unsigned char)(o->window.y>>8);
    r[14]=(unsigned char)o->window.pattern;r[15]=(unsigned char)o->window.dragging;
    r[16]=(unsigned char)o->counter.presses;
}

void start(void)
{
    cs_surface surface;cs_cursor_state cursor;cs_cursor_token token;cs_cursor_bits bits;
    cs_cursor_art art;cs_window_config config;cs_product_state product;cs_window_input owner;
    cs_pointer_sample sample;
#ifdef CS_P286_RAW_KEYBOARD
    cs_pc286_set2_pointer keyboard;unsigned status,flags_before,flags_after,mask_before,mask_after;
#else
    cs_pc286_keyboard_pointer keyboard;unsigned key;
#endif
    unsigned count=0;int result,old_x,old_y;
    config.x=80;config.y=70;config.width=224;config.height=144;config.title=18;
    product.value=97;product.inputs=0;product.progress=16;product.phases=6;product.success=1;product.pixels=0;
    if(cs_surface_init(&surface,canvas,sizeof(canvas),512,342,64,CS_SURFACE_MONO1_MSB)!=CS_SURFACE_OK)fail(0xe0);
    cs_window_cursor_bits_init(&bits);art.bits=&bits;art.width=15;art.height=15;art.hot_x=7;art.hot_y=7;
    if(cs_cursor_init(&cursor)!=CS_CURSOR_OK || cs_cursor_bind(&cursor,&surface,saved,sizeof(saved),&token)!=CS_CURSOR_OK)fail(0xe1);
    if(cs_window_input_begin(&owner,&cursor,token,&art,&config,&product,0)!=CS_WINDOW_INPUT_OK)fail(0xe2);
#ifdef CS_P286_RAW_KEYBOARD
    if(!cs_pc286_set2_begin(&keyboard))fail(0xe6);
#else
    if(!cs_pc286_keyboard_pointer_begin(&keyboard))fail(0xe6);
#endif
#ifdef CS_P286_PREPOSITION_TEST
    sample.dx=-156;sample.dy=-91;sample.buttons=0;
    result=cs_window_input_sample(&owner,&sample);
    if(result!=CS_WINDOW_INPUT_OK && result!=CS_WINDOW_INPUT_CHANGED)fail(0xe3);
#endif
    video_mode();white_display();present(0,0,512,342);
#ifdef CS_P286_RAW_KEYBOARD
    /* Experimental polling owns IRQ1; other interrupt mask bits and the
     * prior interrupt-enable state are preserved. Set-1 is a precondition. */
#ifdef CS_P286_IRQ_TEST_UNMASK
    test_unmask_keyboard_irq();
#endif
    flags_before=flags_word();mask_before=irq_mask();mask_keyboard_irq();
    flags_after=flags_word();mask_after=irq_mask();
    publish_irq(flags_before,flags_after,mask_before,mask_after);
#endif
    publish(&owner,count,keyboard.held);post(0xb0);
    for(;;) {
#ifdef CS_P286_RAW_KEYBOARD
        do {status=key_status();} while(!(status&1u));
        if(status&0xe0u)fail(0xe7);
        result=cs_pc286_set1_feed(&keyboard,key_byte(),&sample);
        if(result<0)fail(0xe8);
        if(!result)continue;
#else
        key=read_key();
        if(!cs_pc286_keyboard_pointer_key(&keyboard,(uint16_t)key,&sample))continue;
#endif
        old_x=(int)owner.pointer.x;old_y=(int)owner.pointer.y;
#ifdef CS_P286_TIMING_MARKERS
        post((unsigned char)(0xc0u+count));
#endif
        result=cs_window_input_sample(&owner,&sample);
#ifdef CS_P286_TIMING_MARKERS
        post((unsigned char)(0xd0u+count));
#endif
        if(result!=CS_WINDOW_INPUT_OK && result!=CS_WINDOW_INPUT_CHANGED)fail(0xe4);
        present((int)owner.render.dirty.left,(int)owner.render.dirty.top,
                (int)owner.render.dirty.right,(int)owner.render.dirty.bottom);
        /* A cursor-only move can report an empty scene dirty rectangle. The
         * saved old and new cursor footprints still need EGA publication. */
        present(old_x-7,old_y-7,old_x+8,old_y+8);
        present((int)owner.pointer.x-7,(int)owner.pointer.y-7,
                (int)owner.pointer.x+8,(int)owner.pointer.y+8);
#ifdef CS_P286_TIMING_MARKERS
        post((unsigned char)(0xe0u+count));
#endif
        ++count;publish(&owner,count,keyboard.held);post((unsigned char)(0xb0u+(count&0x3fu)));
    }
}
