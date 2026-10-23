/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/mac128k/window-session.h"
#include <stdio.h>
#include <string.h>
static unsigned calls,fail_at;
static int result(void){++calls;return calls==fail_at?77:0;}
cs_surface_result cs_surface_init(cs_surface *s,uint8_t *p,size_t n,int32_t w,int32_t h,size_t stride,cs_surface_format f)
{(void)s;(void)p;(void)n;(void)w;(void)h;(void)stride;(void)f;return (cs_surface_result)result();}
cs_cursor_result cs_cursor_init(cs_cursor_state *s)
{(void)s;return (cs_cursor_result)result();}
cs_cursor_result cs_cursor_bind(cs_cursor_state *s,const cs_surface *f,uint8_t *p,size_t n,cs_cursor_token *t)
{(void)s;(void)f;(void)p;(void)n;t->binding=1;t->revision=1;return (cs_cursor_result)result();}
int cs_window_input_begin(cs_window_input *s,cs_cursor_state *c,cs_cursor_token t,const cs_cursor_art *a,const cs_window_config *w,const cs_product_state *p,uint32_t b)
{(void)s;(void)c;(void)t;(void)a;(void)w;(void)p;(void)b;return result();}
int cs_foreground_begin(cs_window_foreground *s,cs_window_inbox *i,cs_window_capture *c,cs_window_input *o,const cs_foreground_hooks *h)
{(void)s;(void)i;(void)c;(void)o;(void)h;return result();}
static uint32_t enter(void *p){(void)p;return 0;}
static void leave(void *p,uint32_t t){(void)p;(void)t;}
int main(void)
{
    static cs_mac_window_session s;
    static uint8_t canvas[CS_MAC_WINDOW_BYTES];
    const cs_foreground_hooks h={enter,leave,NULL};
    const uint32_t stages[]={CS_MAC_WINDOW_STAGE_SURFACE,CS_MAC_WINDOW_STAGE_CURSOR_INIT,
        CS_MAC_WINDOW_STAGE_CURSOR_BIND,CS_MAC_WINDOW_STAGE_INPUT,CS_MAC_WINDOW_STAGE_FOREGROUND};
    for(unsigned i=1;i<=5;++i){
        memset(&s,0xA5,sizeof s);calls=0;fail_at=i;
        if(cs_mac_window_begin(&s,canvas,sizeof canvas,&h)!=CS_MAC_WINDOW_FAULT || calls!=i ||
            s.magic!=CS_MAC_WINDOW_MAGIC || s.phase!=CS_MAC_WINDOW_FAILED || s.stage!=stages[i-1] || s.cause!=77)return 1;
    }
    puts("Mac RAM bootstrap: five retained fault prefixes; no automatic retry.");return 0;
}
