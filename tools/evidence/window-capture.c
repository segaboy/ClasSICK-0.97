/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original hidden-client capture observer. */
#include "../../platform/windows/window-preview.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"capture line%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static const char *directory;
static LPARAM point(int x,int y)
{return (LPARAM)(uintptr_t)((uint32_t)(uint16_t)x|((uint32_t)(uint16_t)y<<16));}
static void event(cs_win_window *s,UINT msg,int x,int y,int down)
{
 RECT r;CHECK(GetClientRect(s->window,&r));int ox=r.right>640?(r.right-640)/2:0,oy=r.bottom>480?(r.bottom-480)/2:0;
 SendMessageW(s->window,msg,down?MK_LBUTTON:0,point(x+ox+64,y+oy+69));
 CHECK(!s->failed && s->input.phase==CS_WINDOW_INPUT_READY);
}
static void capture(cs_win_window *s,const char *name)
{
 RECT r;CHECK(GetClientRect(s->window,&r));BITMAPINFO info={0};void *pixels=NULL;
 info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=r.right;info.bmiHeader.biHeight=-r.bottom;
 info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
 HDC dc=CreateCompatibleDC(NULL);CHECK(dc);HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);CHECK(bitmap && pixels);
 HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old && GetObjectType(old)==OBJ_BITMAP);
 SendMessageW(s->window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);CHECK(GdiFlush() && !s->failed);
 char path[1024];int n=snprintf(path,sizeof(path),"%s/%s.ppm",directory,name);CHECK(n>0 && (size_t)n<sizeof(path));
 FILE *file=fopen(path,"wb");CHECK(file);CHECK(fprintf(file,"P6\n%ld %ld\n255\n",r.right,r.bottom)>0);
 int ox=r.right>640?(r.right-640)/2:0,oy=r.bottom>480?(r.bottom-480)/2:0;
 for(int y=0;y<r.bottom;++y)for(int x=0;x<r.right;++x){
  const uint8_t *b=(const uint8_t*)pixels+((size_t)y*(size_t)r.right+(size_t)x)*4u;int fx=x-ox,fy=y-oy,black=0;
  if(fx>=0 && fx<640 && fy>=0 && fy<480)black=(s->frame[(size_t)fy*80u+(size_t)fx/8u]&(128u>>((unsigned)fx&7u)))!=0;
  CHECK(b[0]==(black?0u:255u) && b[1]==b[0] && b[2]==b[0]);
  uint8_t rgb[3]={b[2],b[1],b[0]};CHECK(fwrite(rgb,1,3,file)==3u);
 }
 CHECK(fclose(file)==0);CHECK(SelectObject(dc,old)==bitmap);CHECK(DeleteObject(bitmap));CHECK(DeleteDC(dc));
}
int main(int argc,char **argv)
{
 if(argc!=2)return 2;
 directory=argv[1];cs_win_window s={0};CHECK(cs_win_window_open(&s,0,0u));
 event(&s,WM_LBUTTONDOWN,256,171,1);event(&s,WM_LBUTTONUP,256,171,0);CHECK(s.input.counter.presses==1u && s.input.window.pattern==1u);
 event(&s,WM_LBUTTONDOWN,100,75,1);
 for(int i=0;i<49;++i){
  int t=i<=24?i:48-i,x=100+t*170/24,y=75+t*60/24;
  event(&s,WM_MOUSEMOVE,x,y,1);CHECK(s.input.counter.presses==2u && s.input.window.dragging && s.input.window.x==x-20 && s.input.window.y==y-5);
  char name[32];CHECK(snprintf(name,sizeof(name),"drag-%02d",i)>0);capture(&s,name);
 }
 event(&s,WM_MOUSEMOVE,270,135,1);event(&s,WM_LBUTTONUP,270,135,0);CHECK(s.input.window.x==250 && s.input.window.y==130 && !s.input.window.dragging);
 capture(&s,"hosted-window-dragged");
 RECT r={0,0,1280,960};CHECK(AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW,FALSE));CHECK(SetWindowPos(s.window,NULL,0,0,r.right-r.left,r.bottom-r.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE));
 event(&s,WM_LBUTTONUP,350,200,0);event(&s,WM_LBUTTONDOWN,350,200,1);event(&s,WM_LBUTTONUP,350,200,0);
 CHECK(s.input.counter.presses==3u && s.input.window.pattern==0u);capture(&s,"hosted-window-large");
 cs_win_window_close(&s);puts("Capture PASS:49 drag frames,2 stills;every GDI pixel matches native frame;synthetic only");return 0;
}
