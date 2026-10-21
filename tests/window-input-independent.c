/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Frozen-contract integration and fault oracle. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/window-scene/input.h"
static unsigned checks,frames,mouse_calls,window_calls,render_calls;
static unsigned fail_mouse,fail_window,fail_render,advanced_failure;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
int cs_fault_input_begin(cs_window_input*,cs_cursor_state*,cs_cursor_token,const cs_cursor_art*,const cs_window_config*,const cs_product_state*,uint32_t);
int cs_fault_input_sample(cs_window_input*,const cs_pointer_sample*);
int cs_fault_input_cancel(cs_window_input*);
int cs_fault_input_repair(cs_window_input*,cs_cursor_token);
int cs_fault_mouse_sample(cs_mouse_button *b,uint32_t down)
{ ++mouse_calls;return mouse_calls==fail_mouse?CS_MOUSE_STATE:cs_mouse_button_sample(b,down); }
int cs_fault_window_apply(cs_window *w0,const cs_pointer_event *e,cs_window_effect *out)
{ ++window_calls;return window_calls==fail_window?CS_WINDOW_STATE:cs_window_apply(w0,e,out); }
int cs_fault_compose(cs_cursor_state *s,cs_cursor_token tok,const cs_cursor_art *a,const cs_product_state *p0,const cs_window *old0,const cs_window *w0,uint32_t inputs,int32_t x,int32_t y,cs_window_scene_effect *out)
{
    ++render_calls;
    if(render_calls==fail_render)return CS_WINDOW_SCENE_ERR_PAINT;
    int rc=cs_window_scene_compose(s,tok,a,p0,old0,w0,inputs,x,y,out);
    if(rc==0 && render_calls==advanced_failure){
        CHECK(cs_cursor_hide(s,out->token)==CS_CURSOR_OK);
        out->stage=CS_WINDOW_SCENE_ADVANCED;out->cause=CS_CURSOR_ERR_ART;return CS_WINDOW_SCENE_ERR_CURSOR;
    }
    return rc;
}
static void pixel(const cs_surface *s,int32_t x,int32_t y,int black)
{
    if(x<0 || x>=s->width || y<0 || y>=s->height)return;
    uint8_t *v=s->storage+(size_t)y*s->stride;
    if(s->format==CS_SURFACE_MONO1_MSB){
        uint8_t mask=(uint8_t)(128u>>((unsigned)x&7u));v+=(size_t)x/8u;
        *v=black?(uint8_t)(*v|mask):(uint8_t)(*v&(uint8_t)~mask);
    }else{v+=(size_t)x*4u;v[0]=black?0u:255u;v[1]=v[0];v[2]=v[0];v[3]=255u;}
}
static void expected(const cs_surface *s,const cs_product_state *p,const cs_window *w,const cs_cursor_art *art,int32_t px,int32_t py)
{
    int32_t ox=s->width>512?(s->width-512)/2:0,oy=s->height>342?(s->height-342)/2:0;
    CHECK(cs_product_scene(s,s->width,s->height,0,p));
    for(int32_t y=0;y<w->height;++y)for(int32_t x=0;x<w->width;++x){
        int black=x==0 || x==w->width-1 || y==0 || y==w->height-1 || y==w->title-1;
        if(!black && y>=w->title)black=w->pattern?((unsigned)x>>2)&1u:(((unsigned)x>>3)^((unsigned)(y-w->title)>>3))&1u;
        pixel(s,ox+w->x+x,oy+w->y+y,black);
    }
    for(unsigned y=0;y<art->height;++y)for(unsigned x=0;x<art->width;++x){
        int32_t lx=px-(int32_t)art->hot_x+(int32_t)x,ly=py-(int32_t)art->hot_y+(int32_t)y;
        unsigned index=y*2u+x/8u,mask=128u>>(x&7u);
        if(lx>=0 && lx<512 && ly>=0 && ly<342 && (art->bits->mask[index]&mask))pixel(s,ox+lx,oy+ly,(art->bits->image[index]&mask)!=0u);
    }
}

typedef struct {
    cs_window_input owner;cs_cursor_state cursor;cs_surface actual,ref;
    cs_cursor_bits bits;cs_cursor_art art;uint8_t payload[1024];
    uint8_t *a,*b;size_t bytes;int injected;
} fixture;
static void setup(fixture *f,int injected,uint32_t buttons,cs_surface_format format)
{
    const cs_window_config cfg={80,70,224,144,18};
    const cs_product_state base={97u,0u,16u,6u,1u,NULL};
    int32_t width=format==CS_SURFACE_MONO1_MSB?512:640,height=format==CS_SURFACE_MONO1_MSB?342:480;
    size_t stride=(format==CS_SURFACE_MONO1_MSB?(size_t)width/8u:(size_t)width*4u)+7u;
    cs_cursor_token token;
    memset(f,0,sizeof(*f));f->injected=injected;f->bytes=stride*(size_t)height+17u;
    f->a=malloc(f->bytes+128u);f->b=malloc(f->bytes+128u);CHECK(f->a && f->b);
    memset(f->a,0xA5,f->bytes+128u);memset(f->b,0xA5,f->bytes+128u);
    f->bits.mask[0]=f->bits.mask[2]=0xC0u;f->bits.image[0]=0x80u;f->bits.image[2]=0x40u;
    f->art.bits=&f->bits;f->art.width=2u;f->art.height=2u;f->art.hot_x=0u;f->art.hot_y=0u;
    CHECK(cs_surface_init(&f->actual,f->a+64u,f->bytes,width,height,stride,format)==CS_SURFACE_OK);
    CHECK(cs_surface_init(&f->ref,f->b+64u,f->bytes,width,height,stride,format)==CS_SURFACE_OK);
    CHECK(cs_cursor_init(&f->cursor)==CS_CURSOR_OK);
    CHECK(cs_cursor_bind(&f->cursor,&f->actual,f->payload,sizeof(f->payload),&token)==CS_CURSOR_OK);
    CHECK((injected?cs_fault_input_begin(&f->owner,&f->cursor,token,&f->art,&cfg,&base,buttons):
        cs_window_input_begin(&f->owner,&f->cursor,token,&f->art,&cfg,&base,buttons))==CS_WINDOW_INPUT_OK);
    mouse_calls=window_calls=render_calls=0u;
}
static void image(fixture *f)
{
    CHECK(f->owner.phase==CS_WINDOW_INPUT_READY && f->owner.stage==CS_WINDOW_INPUT_RENDERED);
    CHECK(f->owner.pointer.down==f->owner.counter.down && f->owner.window.down==f->owner.pointer.down);
    CHECK(f->owner.product.inputs==f->owner.counter.presses && f->owner.rendered_inputs==f->owner.product.inputs);
    expected(&f->ref,&f->owner.product,&f->owner.window,&f->art,f->owner.pointer.x,f->owner.pointer.y);
    CHECK(memcmp(f->a,f->b,f->bytes+128u)==0);++frames;
}
static int sample(fixture *f,int32_t dx,int32_t dy,uint32_t buttons)
{
    cs_pointer_sample s={dx,dy,buttons};return f->injected?cs_fault_input_sample(&f->owner,&s):cs_window_input_sample(&f->owner,&s);
}
static void retire(fixture *f)
{
    cs_cursor_info q;CHECK(cs_cursor_query(&f->cursor,&q)==CS_CURSOR_OK);
    if(q.phase==CS_CURSOR_VISIBLE)CHECK(cs_cursor_hide(&f->cursor,q.token)==CS_CURSOR_OK);
    CHECK(cs_cursor_unbind(&f->cursor,q.token)==CS_CURSOR_OK);free(f->a);free(f->b);
}
static void normal_trace(cs_surface_format format)
{
    fixture f;setup(&f,0,1u,format);image(&f);
    CHECK(sample(&f,0,0,1u)==CS_WINDOW_INPUT_OK);CHECK(f.owner.counter.presses==0u);image(&f);
    CHECK(sample(&f,0,0,0u)==CS_WINDOW_INPUT_CHANGED);image(&f);
    CHECK(sample(&f,0,0,1u)==CS_WINDOW_INPUT_CHANGED);CHECK(f.owner.counter.presses==1u && f.owner.window.pattern==1u);image(&f);
    CHECK(sample(&f,0,0,1u)==CS_WINDOW_INPUT_OK);CHECK(f.owner.counter.presses==1u);image(&f);
    CHECK(sample(&f,-156,-96,0u)==CS_WINDOW_INPUT_CHANGED);image(&f);
    CHECK(sample(&f,0,0,1u)==CS_WINDOW_INPUT_CHANGED);CHECK(f.owner.window.dragging && f.owner.counter.presses==2u);image(&f);
    CHECK(sample(&f,300,175,1u)==CS_WINDOW_INPUT_CHANGED);CHECK(f.owner.window.x==288 && f.owner.window.y==198);image(&f);
    CHECK(sample(&f,-110,-45,0u)==CS_WINDOW_INPUT_CHANGED);CHECK(f.owner.window.x==270 && f.owner.window.y==198 && !f.owner.window.dragging);image(&f);
    CHECK(sample(&f,0,0,1u)==CS_WINDOW_INPUT_CHANGED);CHECK(f.owner.window.dragging && f.owner.counter.presses==3u);image(&f);
    CHECK(sample(&f,INT32_MIN,INT32_MIN,1u)==CS_WINDOW_INPUT_CHANGED);CHECK(f.owner.window.x==0 && f.owner.window.y==0);image(&f);
    CHECK(cs_window_input_cancel(&f.owner)==CS_WINDOW_INPUT_CHANGED);CHECK(!f.owner.window.dragging && f.owner.pointer.suppress==1u && f.owner.counter.presses==3u);image(&f);
    CHECK(sample(&f,0,0,1u)==CS_WINDOW_INPUT_OK);CHECK(f.owner.counter.presses==3u);image(&f);
    CHECK(sample(&f,0,0,0u)==CS_WINDOW_INPUT_CHANGED);image(&f);
    CHECK(sample(&f,50,50,1u)==CS_WINDOW_INPUT_CHANGED);CHECK(f.owner.counter.presses==4u && f.owner.window.pattern==0u);image(&f);
    CHECK(sample(&f,0,0,6u)==CS_WINDOW_INPUT_CHANGED);image(&f);
    CHECK(sample(&f,0,0,6u)==CS_WINDOW_INPUT_OK);image(&f);
    CHECK(sample(&f,461,291,1u)==CS_WINDOW_INPUT_CHANGED);CHECK(f.owner.counter.presses==5u && !f.owner.window.dragging);image(&f);
    CHECK(sample(&f,0,0,0u)==CS_WINDOW_INPUT_CHANGED);image(&f);
    cs_window_input snapshot=f.owner;cs_cursor_state cursor=f.cursor;uint8_t saved[1024];memcpy(saved,f.payload,sizeof(saved));
    CHECK(sample(&f,0,0,8u)==CS_WINDOW_INPUT_ARGUMENT);
    CHECK(memcmp(&snapshot,&f.owner,sizeof(snapshot))==0 && memcmp(&cursor,&f.cursor,sizeof(cursor))==0 && memcmp(saved,f.payload,sizeof(saved))==0);image(&f);
    CHECK(cs_window_input_sample(&f.owner,(const cs_pointer_sample*)(const void*)&f.owner)==CS_WINDOW_INPUT_ARGUMENT);
    CHECK(cs_window_input_repair(&f.owner,f.owner.token)==CS_WINDOW_INPUT_STATE);
    CHECK(memcmp(&snapshot,&f.owner,sizeof(snapshot))==0);image(&f);
    /* Synthetic,coherent saturation fixture;render MAX before testing its edge. */
    f.owner.counter.presses=UINT32_MAX;f.owner.product.inputs=UINT32_MAX;
    CHECK(cs_window_scene_compose(&f.cursor,f.owner.token,&f.art,&f.owner.product,&f.owner.rendered_window,&f.owner.window,f.owner.rendered_inputs,f.owner.pointer.x,f.owner.pointer.y,&f.owner.render)==CS_WINDOW_SCENE_OK);
    f.owner.token=f.owner.render.token;f.owner.rendered_inputs=UINT32_MAX;
    CHECK(sample(&f,-461,-291,1u)==CS_WINDOW_INPUT_CHANGED);CHECK(f.owner.counter.presses==UINT32_MAX && f.owner.window.pattern==1u);image(&f);
    CHECK(cs_window_input_cancel(&f.owner)==CS_WINDOW_INPUT_CHANGED);image(&f);retire(&f);
}
static void faults(void)
{
    for(unsigned which=1u;which<=5u;++which){
        fixture f;fail_mouse=fail_window=fail_render=advanced_failure=0u;setup(&f,1,0u,CS_SURFACE_MONO1_MSB);
        cs_cursor_token before=f.owner.token;
        if(which==1u || which==5u)fail_mouse=1u;
        if(which==2u)fail_window=1u;
        if(which==3u)fail_render=1u;
        if(which==4u)advanced_failure=1u;
        CHECK(sample(&f,0,0,1u)==CS_WINDOW_INPUT_FAULT);
        CHECK(f.owner.phase==CS_WINDOW_INPUT_FAULTED && f.owner.pointer.down==1u);
        CHECK(f.owner.stage==(which==1u || which==5u?CS_WINDOW_INPUT_POINTER:which==2u?CS_WINDOW_INPUT_COUNTER:CS_WINDOW_INPUT_WINDOW));
        CHECK(f.owner.counter.presses==((which==1u || which==5u)?0u:1u));
        if(which==3u)CHECK(f.owner.render.stage==0u && f.owner.token.revision==before.revision);
        if(which==4u)CHECK(f.owner.render.stage==CS_WINDOW_SCENE_ADVANCED && f.owner.token.revision==before.revision+1u);
        cs_window_input frozen=f.owner;
        CHECK(sample(&f,0,0,0u)==CS_WINDOW_INPUT_STATE);CHECK(cs_fault_input_cancel(&f.owner)==CS_WINDOW_INPUT_STATE);
        CHECK(memcmp(&frozen,&f.owner,sizeof(frozen))==0);
        if(which==5u){
            fail_mouse=3u;
            CHECK(cs_fault_input_repair(&f.owner,f.owner.token)==CS_WINDOW_INPUT_FAULT);
            CHECK(f.owner.stage==CS_WINDOW_INPUT_POINTER && f.owner.pointer.down==0u && f.owner.counter.down==1u);
            CHECK(f.owner.counter.presses==1u && f.owner.window.pattern==1u);
        }
        CHECK(cs_fault_input_repair(&f.owner,f.owner.token)==CS_WINDOW_INPUT_OK);
        CHECK(f.owner.counter.presses==1u && f.owner.window.pattern==1u && f.owner.pointer.suppress==1u && !f.owner.window.dragging);image(&f);
        CHECK(sample(&f,0,0,1u)==CS_WINDOW_INPUT_OK);CHECK(f.owner.counter.presses==1u);image(&f);
        CHECK(sample(&f,0,0,0u)==CS_WINDOW_INPUT_CHANGED);CHECK(sample(&f,0,0,1u)==CS_WINDOW_INPUT_CHANGED);
        CHECK(f.owner.counter.presses==2u && f.owner.window.pattern==0u);image(&f);retire(&f);
    }
    fail_mouse=fail_window=fail_render=advanced_failure=0u;
}
static void rebind_recovery(void)
{
    fixture f;setup(&f,1,0u,CS_SURFACE_MONO1_MSB);fail_mouse=1u;
    CHECK(sample(&f,0,0,1u)==CS_WINDOW_INPUT_FAULT);
    cs_cursor_token old=f.owner.token,new_token;
    CHECK(cs_cursor_hide(&f.cursor,old)==CS_CURSOR_OK);
    CHECK(cs_cursor_unbind(&f.cursor,old)==CS_CURSOR_OK);
    CHECK(cs_cursor_bind(&f.cursor,&f.actual,f.payload,sizeof(f.payload),&new_token)==CS_CURSOR_OK);
    CHECK(new_token.binding!=old.binding);fail_mouse=2u;
    CHECK(cs_fault_input_repair(&f.owner,new_token)==CS_WINDOW_INPUT_FAULT);
    CHECK(f.owner.token.binding==new_token.binding && f.owner.counter.presses==0u);
    CHECK(cs_fault_input_repair(&f.owner,new_token)==CS_WINDOW_INPUT_OK);
    CHECK(f.owner.counter.presses==1u && f.owner.window.pattern==1u);image(&f);retire(&f);
    fail_mouse=0u;
}
static void begin_refusal_and_fault(void)
{
    fixture f;setup(&f,1,0u,CS_SURFACE_MONO1_MSB);
    const cs_window_config cfg={80,70,224,144,18};
    const cs_product_state base={97u,0u,16u,6u,1u,NULL};
    cs_window_input fresh,saved;memset(&fresh,0x5A,sizeof(fresh));memcpy(&saved,&fresh,sizeof(saved));
    CHECK(cs_cursor_hide(&f.cursor,f.owner.token)==CS_CURSOR_OK);
    cs_cursor_art bad=f.art;bad.width=0u;
    CHECK(cs_window_input_begin(&fresh,&f.cursor,f.owner.token,&bad,&cfg,&base,0u)==CS_WINDOW_INPUT_ARGUMENT);
    CHECK(memcmp(&saved,&fresh,sizeof(saved))==0);
    CHECK(cs_window_input_begin(&fresh,&f.cursor,f.owner.token,&f.art,(const cs_window_config*)(const void*)&fresh,&base,0u)==CS_WINDOW_INPUT_ARGUMENT);
    CHECK(memcmp(&saved,&fresh,sizeof(saved))==0);
    fail_render=1u;
    CHECK(cs_fault_input_begin(&fresh,&f.cursor,f.owner.token,&f.art,&cfg,&base,0u)==CS_WINDOW_INPUT_FAULT);
    CHECK(fresh.stage==CS_WINDOW_INPUT_WINDOW && fresh.render.stage==0u && fresh.counter.presses==0u);
    CHECK(cs_fault_input_repair(&fresh,fresh.token)==CS_WINDOW_INPUT_OK);
    CHECK(fresh.pointer.suppress==1u && fresh.counter.presses==0u);
    /* The old session is quiescent and retired;check the new live owner in place. */
    expected(&f.ref,&fresh.product,&fresh.window,&f.art,fresh.pointer.x,fresh.pointer.y);
    CHECK(memcmp(f.a,f.b,f.bytes+128u)==0);++frames;
    retire(&f);fail_render=0u;
}
int main(void)
{
    _Static_assert(sizeof(cs_window_input)==(sizeof(void*)==8u?248u:224u),"owner ABI");
    normal_trace(CS_SURFACE_MONO1_MSB);normal_trace(CS_SURFACE_RGBA8);faults();rebind_recovery();begin_refusal_and_fault();
    printf("Input owner independent PASS:%u frames,%u checks\n",frames,checks);return 0;
}
