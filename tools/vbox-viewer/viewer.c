/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original diagnostic, SPEC-0098. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "motion.h"
#include "viewer-target.h"
static HWND window;
static FILE *events;
static unsigned char *frame,*staging;
static cs_view_motion motion;
static unsigned event_seq;
static uint32_t frame_seq;
static ULONGLONG began,fresh;
static int failed,releasing,closing,last_guard=-1;
static const wchar_t klass[]=L"ClasSICKDiagnosticViewer1";
static int desktop_ok(void)
{
    wchar_t name[32];DWORD need=0;
    HDESK d=OpenInputDesktop(0,FALSE,DESKTOP_READOBJECTS);
    int ok;
    if(!d)return 0;
    ok=GetUserObjectInformationW(d,UOI_NAME,name,sizeof name,&need)&&!wcscmp(name,L"Default");
    CloseDesktop(d);return ok;
}
static void fail(void)
{failed=1;if(window&&!closing)PostMessageW(window,WM_CLOSE,0,0);}
static void emit(int kind,const cs_view_event *e)
{
    if(kind<=0)return;
    if(!events || event_seq>=4096 || (event_seq==4095 && (kind!=2 || e->buttons))) {fail();return;}
    ++event_seq;
    if(fprintf(events,"{\"seq\":%u,\"tick_ms\":%llu,\"kind\":%d,\"dx\":%d,\"dy\":%d,\"buttons\":%d}\n",
        event_seq,(unsigned long long)GetTickCount64(),kind,e->dx,e->dy,e->buttons)<0 || fflush(events))fail();
}
static void release_capture(void)
{
    if(window&&GetCapture()==window){releasing=1;ReleaseCapture();releasing=0;}
}
static void cancel(void)
{cs_view_event e;int r=cs_view_cancel(&motion,&e);emit(r,&e);release_capture();}
static int frame_ready(void)
{return frame_seq && GetTickCount64()-fresh<2000;}
static void refresh(void)
{
    FILE *f;size_t n;uint32_t seq=0;int end,err,guard;
    if((f=fopen(CS_VIEWER_ROOT "/stop.request","rb"))!=NULL){fclose(f);PostMessageW(window,WM_CLOSE,0,0);return;}
    if(GetTickCount64()-began>=300000){PostMessageW(window,WM_CLOSE,0,0);return;}
    f=fopen(CS_VIEWER_ROOT "/frame.bin","rb");
    if(f){
        n=fread(staging,1,CS_VIEW_FRAME_SIZE,f);end=fgetc(f);err=ferror(f);
        if(fclose(f))err=1;
        if(!err&&end==EOF)seq=cs_view_frame(staging,n,frame_seq);
    }
    if(seq){
        unsigned char *tmp=frame;frame=staging;staging=tmp;frame_seq=seq;fresh=GetTickCount64();
        InvalidateRect(window,NULL,FALSE);
    }
    guard=(frame_ready()?1:0)|(GetForegroundWindow()==window?2:0)|(desktop_ok()?4:0);
    if(guard!=last_guard){printf("guard_bits=%d frame_seq=%u tick_ms=%llu\n",guard,frame_seq,(unsigned long long)GetTickCount64());fflush(stdout);last_guard=guard;}
    if(guard!=7)cancel();
}
static void mouse(UINT msg,WPARAM wp,LPARAM lp)
{
    int x=(int)(int16_t)(lp&0xffff),y=(int)(int16_t)((lp>>16)&0xffff);
    int held=msg==WM_LBUTTONDOWN?1:msg==WM_LBUTTONUP?0:!!(wp&MK_LBUTTON);
    int inside=x>=0&&x<1024&&y>=0&&y<768;
    int enabled=!failed&&frame_ready()&&GetForegroundWindow()==window&&desktop_ok();
    cs_view_event e;int r;
    if(!enabled){cancel();return;}
    r=cs_view_step(&motion,x,y,inside||GetCapture()==window,held,enabled,msg==WM_LBUTTONDOWN,&e);
    if(r<0){cancel();fail();return;}
    if(motion.held&&GetCapture()!=window){
        SetCapture(window);
        if(GetCapture()!=window){cancel();fail();return;}
    }
    emit(r,&e);
    if(!motion.held)release_capture();
    if(inside){
        TRACKMOUSEEVENT t={sizeof t,TME_LEAVE,window,0};
        if(!TrackMouseEvent(&t))fail();
    }
}
static void paint(HDC dc)
{
    BITMAPINFO bi={0};RECT rc={0,0,1024,768};int rows;
    if(!frame_seq){FillRect(dc,&rc,(HBRUSH)GetStockObject(WHITE_BRUSH));return;}
    bi.bmiHeader.biSize=sizeof bi.bmiHeader;bi.bmiHeader.biWidth=1024;bi.bmiHeader.biHeight=-768;
    bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;
    rows=StretchDIBits(dc,0,0,1024,768,0,0,1024,768,frame+16,&bi,DIB_RGB_COLORS,SRCCOPY);
    if(!rows||(DWORD)rows==GDI_ERROR)fail();
}
static LRESULT CALLBACK proc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
    switch(msg){
    case WM_CLOSE:
        closing=1;cancel();KillTimer(hwnd,1);DestroyWindow(hwnd);return 0;
    case WM_DESTROY: window=NULL;PostQuitMessage(0);return 0;
    case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);paint(dc);EndPaint(hwnd,&ps);return 0;}
    case WM_PRINTCLIENT:paint((HDC)wp);return 0;
    case WM_ERASEBKGND:return 1;
    case WM_TIMER:if(wp==1)refresh();return 0;
    case WM_MOUSEMOVE:case WM_LBUTTONDOWN:case WM_LBUTTONUP:mouse(msg,wp,lp);return 0;
    case WM_CAPTURECHANGED:if(!releasing&&(HWND)lp!=hwnd)cancel();return 0;
    case WM_MOUSELEAVE:if(GetCapture()!=hwnd)cancel();return 0;
    case WM_KILLFOCUS:case WM_CANCELMODE:case WM_SIZE:cancel();break;
    case WM_KEYDOWN:if(wp==VK_ESCAPE){PostMessageW(hwnd,WM_CLOSE,0,0);return 0;}break;
    default:break;
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}
int main(int argc,char **argv)
{
    WNDCLASSEXW wc={0};RECT rc={0,0,1024,768};MSG msg;BOOL r;HINSTANCE hi=GetModuleHandleW(NULL);
    DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
    (void)argv;if(argc!=1||!desktop_ok())return 2;
    if(!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))return 2;
    frame=calloc(1,CS_VIEW_FRAME_SIZE);staging=calloc(1,CS_VIEW_FRAME_SIZE);
    events=fopen(CS_VIEWER_ROOT "/events.jsonl","wx");
    if(!frame||!staging||!events){failed=1;goto done;}
    motion.suppressed=!!(GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON)?VK_RBUTTON:VK_LBUTTON)&0x8000);
    wc.cbSize=sizeof wc;wc.lpfnWndProc=proc;wc.hInstance=hi;wc.hCursor=LoadCursorW(NULL,IDC_ARROW);wc.lpszClassName=klass;
    if(!RegisterClassExW(&wc)||!AdjustWindowRect(&rc,style,FALSE)){failed=1;goto done;}
    began=GetTickCount64();
    window=CreateWindowExW(0,klass,L"ClasSICK PC diagnostic viewer",style,100,100,
        rc.right-rc.left,rc.bottom-rc.top,NULL,NULL,hi,NULL);
    if(!window){failed=1;goto done;}
    if(!SetTimer(window,1,50,NULL)){failed=1;DestroyWindow(window);goto done;}
    ShowWindow(window,SW_SHOW);UpdateWindow(window);
    /* One ordinary request for our own window; no thread attachment or retry. */
    printf("own_foreground_request=%d\n",SetForegroundWindow(window)!=0);fflush(stdout);
    while((r=GetMessageW(&msg,NULL,0,0))>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
    if(r<0)failed=1;
done:
    if(window){cancel();DestroyWindow(window);}
    if(events&&fclose(events))failed=1;
    free(frame);free(staging);return failed?1:0;
}
