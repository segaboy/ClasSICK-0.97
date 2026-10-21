/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Frozen-contract Win32 adapter checks. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/windows/window-preview.h"
static unsigned checks,frames,paints;
#define CHECK(x) do { ++checks;if(!(x)){fprintf(stderr,"line %d:%s\n",__LINE__,#x);exit(1);} } while(0)
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

static void image(cs_win_window *s)
{
    uint8_t reference[80u*480u];cs_surface sf;
    CHECK(cs_surface_init(&sf,reference,sizeof(reference),640,480,80u,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    expected(&sf,&s->input.product,&s->input.window,&s->art,s->input.pointer.x,s->input.pointer.y);
    CHECK(!s->failed && s->input.phase==CS_WINDOW_INPUT_READY);
    CHECK(memcmp(reference,s->frame,sizeof(reference))==0);++frames;
}
static LPARAM position(int x,int y)
{ return (LPARAM)(uintptr_t)((uint32_t)(uint16_t)x|((uint32_t)(uint16_t)y<<16)); }
static void send_at(cs_win_window *s,UINT msg,int x,int y,int down)
{
    RECT r;CHECK(GetClientRect(s->window,&r));
    int ox=r.right>640?(r.right-640)/2:0,oy=r.bottom>480?(r.bottom-480)/2:0;
    SendMessageW(s->window,msg,down?MK_LBUTTON:0,position(x+ox+64,y+oy+69));image(s);
}
static void resize(cs_win_window *s,int width,int height)
{
    RECT r={0,0,width,height};CHECK(AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW,FALSE));
    CHECK(SetWindowPos(s->window,NULL,0,0,r.right-r.left,r.bottom-r.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE));
    RECT actual;CHECK(GetClientRect(s->window,&actual));CHECK(actual.right==width && actual.bottom==height);
}
static void print_client(cs_win_window *s)
{
    RECT r;CHECK(GetClientRect(s->window,&r));size_t size=(size_t)r.right*(size_t)r.bottom*4u;
    BITMAPINFO info={0};void *pixels=NULL;info.bmiHeader.biSize=sizeof(info.bmiHeader);
    info.bmiHeader.biWidth=r.right;info.bmiHeader.biHeight=-r.bottom;info.bmiHeader.biPlanes=1;
    info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);CHECK(bitmap && pixels);
    HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old && GetObjectType(old)==OBJ_BITMAP);memset(pixels,0x55,size);
    unsigned before=s->paints;SendMessageW(s->window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);GdiFlush();CHECK(!s->failed && s->paints==before+1u);
    int ox=r.right>640?(r.right-640)/2:0,oy=r.bottom>480?(r.bottom-480)/2:0;
    for(int y=0;y<r.bottom;++y)for(int x=0;x<r.right;++x){
        int fx=x-ox,fy=y-oy,black=0;
        if(fx>=0 && fx<640 && fy>=0 && fy<480)black=(s->frame[(size_t)fy*80u+(size_t)fx/8u]&(128u>>((unsigned)fx&7u)))!=0;
        const uint8_t *b=(const uint8_t*)pixels+((size_t)y*(size_t)r.right+(size_t)x)*4u;
        CHECK(b[0]==(black?0u:255u) && b[1]==b[0] && b[2]==b[0]);
    }
    CHECK(SelectObject(dc,old)==bitmap);CHECK(DeleteObject(bitmap));CHECK(DeleteDC(dc));++paints;
}
static void discard_quit(void)
{ MSG m;while(PeekMessageW(&m,NULL,WM_QUIT,WM_QUIT,PM_REMOVE)){} }
int main(void)
{
    cs_win_window s={0};CHECK(cs_win_window_open(&s,0,1u));image(&s);print_client(&s);
    send_at(&s,WM_MOUSEMOVE,256,171,1);CHECK(s.input.counter.presses==0u);
    send_at(&s,WM_LBUTTONUP,256,171,0);CHECK(GetCapture()!=s.window);
    send_at(&s,WM_LBUTTONDOWN,256,171,1);CHECK(s.input.counter.presses==1u && s.input.window.pattern==1u && GetCapture()==s.window);
    send_at(&s,WM_LBUTTONDOWN,256,171,1);CHECK(s.input.counter.presses==1u);
    send_at(&s,WM_LBUTTONUP,256,171,0);CHECK(!s.input.pointer.suppress);
    send_at(&s,WM_LBUTTONDOWN,256,171,1);CHECK(s.input.counter.presses==2u && !s.input.window.pattern);
    send_at(&s,WM_LBUTTONUP,256,171,0);
    send_at(&s,WM_LBUTTONDOWN,100,75,1);CHECK(s.input.window.dragging && s.input.counter.presses==3u);
    send_at(&s,WM_MOUSEMOVE,400,250,1);CHECK(s.input.window.x==288 && s.input.window.y==198);
    send_at(&s,WM_LBUTTONUP,290,205,0);CHECK(s.input.window.x==270 && s.input.window.y==198 && !s.input.window.dragging && !s.input.pointer.suppress);
    send_at(&s,WM_LBUTTONDOWN,290,205,1);CHECK(s.input.window.dragging && s.input.counter.presses==4u);
    SendMessageW(s.window,WM_KILLFOCUS,0,0);image(&s);CHECK(!s.input.window.dragging && s.input.pointer.suppress && GetCapture()!=s.window);
    send_at(&s,WM_MOUSEMOVE,290,205,1);CHECK(s.input.counter.presses==4u);
    send_at(&s,WM_LBUTTONUP,290,205,0);
    send_at(&s,WM_LBUTTONDOWN,290,205,1);CHECK(s.input.counter.presses==5u);
    CHECK(ReleaseCapture());image(&s);CHECK(!s.input.window.dragging && s.input.pointer.suppress);
    send_at(&s,WM_LBUTTONUP,290,205,0);
    send_at(&s,WM_LBUTTONDOWN,290,205,1);CHECK(s.input.counter.presses==6u);
    /* Signed lParam and clamp outside client during capture. */
    SendMessageW(s.window,WM_MOUSEMOVE,MK_LBUTTON,position(-20,-30));image(&s);CHECK(s.input.pointer.x==0 && s.input.pointer.y==0 && s.input.window.x==0 && s.input.window.y==0);
    send_at(&s,WM_LBUTTONUP,0,0,0);
    send_at(&s,WM_LBUTTONDOWN,-30,100,1);CHECK(s.input.counter.presses==6u && s.input.pointer.suppress);
    send_at(&s,WM_MOUSEMOVE,50,50,1);CHECK(s.input.counter.presses==6u);
    send_at(&s,WM_LBUTTONUP,50,50,0);send_at(&s,WM_LBUTTONDOWN,50,50,1);CHECK(s.input.counter.presses==7u && s.input.window.pattern==1u);
    SendMessageW(s.window,WM_CANCELMODE,0,0);image(&s);CHECK(s.input.pointer.suppress && !s.input.pointer.down);
    send_at(&s,WM_LBUTTONUP,-30,100,0);CHECK(!s.input.pointer.suppress);
    send_at(&s,WM_LBUTTONDOWN,50,50,1);CHECK(s.input.counter.presses==8u && !s.input.window.pattern);
    send_at(&s,WM_LBUTTONUP,50,50,0);
    for(unsigned k=0;k<2u;++k){resize(&s,k==0u?1280:400,k==0u?960:300);image(&s);print_client(&s);send_at(&s,WM_LBUTTONUP,100,75,0);send_at(&s,WM_LBUTTONDOWN,100,75,1);CHECK(s.input.pointer.x==100 && s.input.pointer.y==75);SendMessageW(s.window,WM_MOUSELEAVE,0,0);CHECK(s.input.pointer.down);send_at(&s,WM_LBUTTONUP,100,75,0);}
    cs_win_window_close(&s);CHECK(!s.window && !s.frame && !s.scratch && !s.live);cs_win_window_close(&s);discard_quit();
    CHECK(cs_win_window_open(&s,0,0u));send_at(&s,WM_LBUTTONDOWN,256,171,1);CHECK(s.input.counter.presses==1u);
    s.input.phase=CS_WINDOW_INPUT_FAULTED;s.input.stage=CS_WINDOW_INPUT_WINDOW;
    SendMessageW(s.window,WM_MOUSEMOVE,MK_LBUTTON,position(164,144));
    CHECK(s.failed && s.input.phase==CS_WINDOW_INPUT_FAULTED && s.input.counter.presses==1u);
    SendMessageW(s.window,WM_LBUTTONDOWN,MK_LBUTTON,position(164,144));CHECK(s.input.counter.presses==1u);
    SendMessageW(s.window,WM_CLOSE,0,0);CHECK(!s.window && !s.live);cs_win_window_close(&s);discard_quit();
    CHECK(cs_win_window_open(NULL,0,0u)==0);
    printf("Hosted window adapter PASS:%u complete frames,%u client paints,%u checks;synthetic messages only\n",frames,paints,checks);return 0;
}
