/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* 286 keyboard and EGA integration experiment using the real shared semantic
 * core. The simple geometric renderer is not the shared preview compositor. */
#include "../../arch/i286/c16.h"
#include "../../core/ui/window.h"

#if !defined(__WATCOMC__) || !defined(_M_I86)
#error Requires the pinned 16-bit Open Watcom target
#endif
CS_C16_ASSERT(cs_window_keyboard_near, sizeof(void *) == 2);

static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void video_mode(void);
#pragma aux video_mode = "mov ax,0010h" "int 10h" modify [ax bx cx dx si di];
static unsigned short read_key(void);
#pragma aux read_key = "xor ah,ah" "int 16h" value [ax] modify [ax bx cx dx si di];
static void fail(unsigned char value) { post(value); for (;;) { } }
static int clip_left,clip_top,clip_right,clip_bottom;

/* Inclusive left/top and exclusive right/bottom on the logical 512x342 plane.
 * EGA's byte 1 is white in this modeled mode. Coordinates are clipped before
 * forming any offset; the complete canvas fits within the 64K aperture. */
static void rect(int left,int top,int right,int bottom,unsigned char white)
{
    unsigned a,b,row,i; unsigned char head,tail,mask;
    unsigned char __far *video=(unsigned char __far *)0xa0000000UL;
    if(left<0)left=0;if(top<0)top=0;if(right>512)right=512;if(bottom>342)bottom=342;
    if(left<clip_left)left=clip_left;if(top<clip_top)top=clip_top;
    if(right>clip_right)right=clip_right;if(bottom>clip_bottom)bottom=clip_bottom;
    if(left>=right || top>=bottom)return;
    a=(unsigned)(left+64)/8u;b=(unsigned)(right+63)/8u;
    head=(unsigned char)(0xffu>>((unsigned)left&7u));
    tail=(unsigned char)(0xffu<<(7u-((unsigned)(right-1)&7u)));
    for(row=(unsigned)top+4u;row<(unsigned)bottom+4u;++row) {
        unsigned base=row*80u;
        for(i=a;i<=b;++i) {
            mask=0xffu;
            if(i==a)mask=(unsigned char)(mask&head);
            if(i==b)mask=(unsigned char)(mask&tail);
            if(white)video[base+i]=(unsigned char)(video[base+i]|mask);
            else video[base+i]=(unsigned char)(video[base+i]&(unsigned char)~mask);
        }
    }
}

static void render(const cs_window *w,const cs_pointer *p,int l,int t,int r,int b)
{
    int x=(int)w->x,y=(int)w->y,px=(int)p->x,py=(int)p->y,i,j;
    clip_left=l;clip_top=t;clip_right=r;clip_bottom=b;
    rect(0,0,512,342,1);
    rect(x,y,x+224,y+144,0);
    rect(x+1,y+1,x+223,y+17,1);
    rect(x+1,y+18,x+223,y+143,1);
    if(w->pattern) {
        for(i=4;i<223;i+=8)rect(x+i,y+18,x+(i+4<223?i+4:223),y+143,0);
    } else {
        for(j=0;j<125;j+=8)
            for(i=0;i<223;i+=8)
                if(((i/8)^(j/8))&1)rect(x+(i?i:1),y+18+j,
                    x+(i+8<223?i+8:223),y+18+(j+8<125?j+8:125),0);
    }
    rect(px-1,py-7,px+2,py+8,1);rect(px-7,py-1,px+8,py+2,1);
    rect(px,py-7,px+1,py+8,0);rect(px-7,py,px+8,py+1,0);
}

static int min(int a,int b) { return a<b?a:b; }
static int max(int a,int b) { return a>b?a:b; }

static void publish(const cs_window *w,const cs_pointer *p,unsigned count,unsigned button)
{
    unsigned char __far *o=(unsigned char __far *)0x60000000UL;
    o[0]='P';o[1]='2';o[2]='8';o[3]='6';o[4]=(unsigned char)count;o[5]=(unsigned char)button;
    o[6]=(unsigned char)p->x;o[7]=(unsigned char)(p->x>>8);
    o[8]=(unsigned char)p->y;o[9]=(unsigned char)(p->y>>8);
    o[10]=(unsigned char)w->x;o[11]=(unsigned char)(w->x>>8);
    o[12]=(unsigned char)w->y;o[13]=(unsigned char)(w->y>>8);
    o[14]=(unsigned char)w->pattern;o[15]=(unsigned char)w->dragging;
}

void start(void)
{
    cs_pointer pointer;cs_window window;cs_window_config config;
    cs_pointer_sample s;cs_pointer_event e;cs_window_effect effect;
    unsigned key,scan,count=0,button=0;int result,ox,oy,wx,wy,pattern,l,t,r,b;
    config.x=80;config.y=70;config.width=224;config.height=144;config.title=18;
    if(cs_pointer_begin(&pointer,0)!=CS_POINTER_OK || cs_window_begin(&window,&config)!=CS_WINDOW_OK)fail(0xe0);
    /* The fixture starts near the title; subsequent input is BIOS keyboard. */
    s.dx=-156;s.dy=-91;s.buttons=0;
    if(cs_pointer_sample_apply(&pointer,&s,&e)!=CS_POINTER_CHANGED ||
       cs_window_apply(&window,&e,&effect)>CS_WINDOW_CHANGED)fail(0xe1);
    video_mode();render(&window,&pointer,0,0,512,342);publish(&window,&pointer,count,button);post(0x90);
    for(;;) {
        key=read_key();scan=key>>8;s.dx=0;s.dy=0;
        if(scan==0x4b)s.dx=-16;else if(scan==0x4d)s.dx=16;
        else if(scan==0x48)s.dy=-16;else if(scan==0x50)s.dy=16;
        else if((key&0xffu)==0x20u)button^=1u;else continue;
        s.buttons=button;ox=(int)pointer.x;oy=(int)pointer.y;
        wx=(int)window.x;wy=(int)window.y;pattern=(int)window.pattern;
        result=cs_pointer_sample_apply(&pointer,&s,&e);
        if(result!=CS_POINTER_OK && result!=CS_POINTER_CHANGED)fail(0xe2);
        result=cs_window_apply(&window,&e,&effect);
        if(result!=CS_WINDOW_OK && result!=CS_WINDOW_CHANGED)fail(0xe3);
        if(ox!=(int)pointer.x || oy!=(int)pointer.y || wx!=(int)window.x ||
           wy!=(int)window.y || pattern!=(int)window.pattern) {
            l=min(ox,(int)pointer.x)-7;t=min(oy,(int)pointer.y)-7;
            r=max(ox,(int)pointer.x)+8;b=max(oy,(int)pointer.y)+8;
            if(wx!=(int)window.x || wy!=(int)window.y || pattern!=(int)window.pattern) {
                l=min(l,min(wx,(int)window.x));t=min(t,min(wy,(int)window.y));
                r=max(r,max(wx,(int)window.x)+224);b=max(b,max(wy,(int)window.y)+144);
            }
            render(&window,&pointer,l,t,r,b);
        }
        ++count;publish(&window,&pointer,count,button);
        post((unsigned char)(0x90u+(count&0x3fu)));
    }
}
