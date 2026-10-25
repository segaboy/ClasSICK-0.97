/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Dean Howell. Original injected native guard tests;no UI input. */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "capture.h"
static int running=1,visible=1,foreground=1,owner=1,default_desktop=1,swap=1;
static int point_inside=1,captured=0,queries=0,change_after_query=0,inputs=0;
static DWORD WINAPI wait_fake(HANDLE h,DWORD ms){(void)h;(void)ms;return running?WAIT_TIMEOUT:WAIT_OBJECT_0;}
static BOOL WINAPI times_fake(HANDLE h,LPFILETIME c,LPFILETIME x,LPFILETIME k,LPFILETIME u){(void)h;(void)x;(void)k;(void)u;c->dwHighDateTime=0;c->dwLowDateTime=42;return TRUE;}
static BOOL WINAPI window_fake(HWND h){(void)h;return TRUE;}
static BOOL WINAPI visible_fake(HWND h){(void)h;return visible;}
static HWND WINAPI fg_fake(void){return (HWND)(UINT_PTR)(foreground?1:9);}
static HWND WINAPI root_fake(HWND h,UINT f){(void)f;return h;}
static DWORD WINAPI pid_fake(HWND h,LPDWORD pid){(void)h;*pid=owner?7:8;return 1;}
static int WINAPI metrics_fake(int n){(void)n;return swap;}
static HDESK WINAPI desktop_fake(DWORD f,BOOL i,ACCESS_MASK a){(void)f;(void)i;(void)a;return (HDESK)(UINT_PTR)1;}
static BOOL WINAPI info_fake(HANDLE h,int n,PVOID p,DWORD z,LPDWORD needed){const wchar_t*s=default_desktop?L"Default":L"Other";size_t bytes=(wcslen(s)+1)*sizeof(wchar_t);(void)h;(void)n;*needed=(DWORD)bytes;if(bytes>z)return FALSE;memcpy(p,s,bytes);return TRUE;}
static BOOL WINAPI close_fake(HDESK h){(void)h;return TRUE;}
static BOOL WINAPI cursor_fake(LPPOINT p){p->x=p->y=10;return TRUE;}
static HWND WINAPI point_fake(POINT p){(void)p;return (HWND)(UINT_PTR)(point_inside?1:3);}
static BOOL WINAPI client_fake(HWND h,LPPOINT p){(void)h;(void)p;return TRUE;}
static BOOL WINAPI rect_fake(HWND h,LPRECT p){(void)h;p->left=p->top=0;p->right=p->bottom=100;return TRUE;}
static UINT WINAPI send_fake(UINT n,LPINPUT p,int size){(void)n;(void)p;(void)size;inputs++;return 0;}
static void capture_fake(void*h,unsigned long p,wh_capture*out){(void)h;(void)p;queries++;out->verified=captured;out->stage="fixture";out->hr=0;if(change_after_query==1)foreground=0;if(change_after_query==2)running=0;if(change_after_query==3)owner=0;if(change_after_query==4)swap=0;if(change_after_query==5)default_desktop=0;}
#define WaitForSingleObject wait_fake
#define GetProcessTimes times_fake
#define IsWindow window_fake
#define IsWindowVisible visible_fake
#define GetForegroundWindow fg_fake
#define GetAncestor root_fake
#define GetWindowThreadProcessId pid_fake
#define GetSystemMetrics metrics_fake
#define OpenInputDesktop desktop_fake
#define GetUserObjectInformationW info_fake
#define CloseDesktop close_fake
#define GetCursorPos cursor_fake
#define WindowFromPoint point_fake
#define ScreenToClient client_fake
#define GetClientRect rect_fake
#define SendInput send_fake
#define WH_CAPTURE_QUERY(h,p,o) capture_fake(h,p,o)
#define main native_cli_not_called
#include "native.c"
#undef main
static unsigned checks,failures;
#define C(x) do{checks++;if(!(x)){failures++;printf("FAIL %u line %d\n",checks,__LINE__);}}while(0)
static void reset(void){static const wih_target t={L"vbox",L"",{0},7,42};g_t=&t;g_hwnd=(HWND)(UINT_PTR)1;g_proc=(HANDLE)(UINT_PTR)1;g_swap=1;running=visible=foreground=owner=default_desktop=swap=point_inside=1;captured=queries=change_after_query=inputs=0;}
int main(void){int i;
 reset();C(guard(NULL)==1);C(queries==0&&inputs==0&&!g_cap.verified);
 reset();point_inside=0;C(guard(NULL)==0);C(queries==1&&!g_cap.verified&&inputs==0);
 reset();point_inside=0;captured=1;C(guard(NULL)==1);C(queries==1&&g_cap.verified&&inputs==0);
 point_inside=1;C(guard(NULL)==1);C(!g_cap.verified&&queries==1);
 for(i=1;i<=5;i++){reset();point_inside=0;captured=1;change_after_query=i;C(guard(NULL)==0);C(queries==1&&inputs==0);}
 reset();foreground=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 reset();running=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 reset();owner=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 reset();default_desktop=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 reset();swap=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 reset();visible=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 printf("native guard %u checks, %u failures;zero native input\n",checks,failures);return failures?1:0;
}
