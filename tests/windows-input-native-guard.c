/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Dean Howell. Original injected native guard tests;no UI input. */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "capture.h"
static int running=1,visible=1,foreground=1,owner=1,default_desktop=1,swap=1;
static int point_inside=1,captured=0,queries=0,change_after_query=0,inputs=0;
static INPUT last_input;
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
static UINT WINAPI send_fake(UINT n,LPINPUT p,int size){(void)n;(void)size;last_input=*p;inputs++;return 0;}
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
static int moves,held,move_ok=1,change_on_move,mv_x,mv_y,mv_w,mv_hh,mv_r;static HWND mv_h;
static BOOL WINAPI move_fake(HWND h,int x,int y,int w,int hh,BOOL r){moves++;mv_h=h;mv_x=x;mv_y=y;mv_w=w;mv_hh=hh;mv_r=r;if(!move_ok)return FALSE;if(change_on_move==1)foreground=0;if(change_on_move==2)default_desktop=0;if(change_on_move==3)swap=0;if(change_on_move==4)running=0;if(change_on_move==5)owner=0;if(change_on_move==6)visible=0;return TRUE;}
static SHORT WINAPI async_fake(int k){return ((held==1&&k==VK_LBUTTON)||(held==2&&k==VK_RBUTTON))?(SHORT)0x8000:0;}
#define MoveWindow move_fake
#define GetAsyncKeyState async_fake
#define WH_CAPTURE_QUERY(h,p,o) capture_fake(h,p,o)
#define WH_TARGET_WINDOW ((UINT_PTR)1)
#define main native_cli_not_called
#include "native.c"
#undef main
static unsigned checks,failures;
#define C(x) do{checks++;if(!(x)){failures++;printf("FAIL %u line %d\n",checks,__LINE__);}}while(0)
static void reset(void){static const wih_target t={L"vbox",L"",{0},7,42};g_t=&t;g_hwnd=(HWND)(UINT_PTR)1;g_proc=(HANDLE)(UINT_PTR)1;g_swap=1;running=visible=foreground=owner=default_desktop=swap=point_inside=1;captured=queries=change_after_query=inputs=0;moves=held=change_on_move=0;move_ok=1;g_place_out="not attempted";}
int main(void){int i,mode;
 {EW e;reset();e.pid=7;e.n=0;e.h=NULL;C(ew((HWND)(UINT_PTR)2,(LPARAM)&e)&&e.n==0);C(ew((HWND)(UINT_PTR)1,(LPARAM)&e)&&e.n==1&&e.h==(HWND)(UINT_PTR)1);owner=0;e.n=0;C(ew((HWND)(UINT_PTR)1,(LPARAM)&e)&&e.n==0);owner=1;visible=0;C(ew((HWND)(UINT_PTR)1,(LPARAM)&e)&&e.n==0);}
 reset();C(guard(NULL)==1);C(queries==0&&inputs==0&&!g_cap.verified);
 reset();point_inside=0;C(guard(NULL)==0);C(queries==1&&!g_cap.verified&&inputs==0);
 reset();point_inside=0;C(guard(NULL)==0&&!strcmp(g_why,"client area"));
 reset();point_inside=0;captured=1;C(guard(NULL)==0);C(queries==1&&g_cap.verified&&inputs==0&&!strcmp(g_why,"client area"));
 point_inside=1;C(guard(NULL)==1);C(!g_cap.verified&&queries==1&&inputs==0);
 for(i=1;i<=5;i++){static const char*why[]={"","foreground","process","owner","mapping","desktop"};reset();point_inside=0;captured=1;change_after_query=i;C(guard(NULL)==0);C(queries==1&&inputs==0&&!strcmp(g_why,why[i]));}
 reset();foreground=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 reset();running=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 reset();owner=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 reset();default_desktop=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 reset();swap=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 reset();visible=0;C(guard(NULL)==0);C(queries==0&&inputs==0);
 for(mode=0;mode<=1;mode++)for(i=0;i<=1;i++){
  DWORD down=(mode||!i)?MOUSEEVENTF_LEFTDOWN:MOUSEEVENTF_RIGHTDOWN;
  DWORD up=(mode||!i)?MOUSEEVENTF_LEFTUP:MOUSEEVENTF_RIGHTUP;
  reset();g_swap=swap=i;g_physical_left=mode;
  C(button_flags('D')==down&&button_flags('U')==up&&button_flags('N')==0);
  (void)send_ev(NULL,-2,3,'D');C(inputs==1&&last_input.type==INPUT_MOUSE&&last_input.mi.dx==-2&&last_input.mi.dy==3&&last_input.mi.dwFlags==(MOUSEEVENTF_MOVE|down));
  (void)send_ev(NULL,0,0,'U');C(inputs==2&&last_input.mi.dwFlags==up);
  C(guard(NULL)==1);swap=!i;C(guard(NULL)==0&&!strcmp(g_why,"mapping"));
 }
 {char *args[]={"helper","--physical-left","--physical-left"};g_physical_left=0;C(native_cli_not_called(3,args)==2);}
 reset();C(place_window()==NULL&&moves==1&&mv_h==g_hwnd&&mv_x==100&&mv_y==100&&mv_w==1024&&mv_hh==854&&mv_r==TRUE&&inputs==0&&queries==0&&!strcmp(g_place_out,"moved"));
 reset();point_inside=0;C(place_window()==NULL&&moves==1&&inputs==0);
 reset();foreground=0;C(place_window()==NULL&&moves==1&&inputs==0);C(guard(NULL)==0&&!strcmp(g_why,"foreground")&&inputs==0);
 reset();running=0;C(place_window()&&moves==0);
 reset();owner=0;C(place_window()&&moves==0);
 reset();visible=0;C(place_window()&&moves==0);
 reset();default_desktop=0;C(place_window()&&moves==0);
 reset();swap=0;C(place_window()&&moves==0);
 for(i=1;i<=2;i++){const char *why;reset();held=i;why=place_window();C(why&&!strcmp(why,"button held")&&moves==0&&inputs==0);}
 reset();move_ok=0;C(place_window()&&moves==1&&inputs==0&&!strcmp(g_place_out,"MoveWindow failed"));
 reset();change_on_move=1;C(place_window()==NULL&&moves==1&&inputs==0);C(guard(NULL)==0&&!strcmp(g_why,"foreground")&&inputs==0);
 for(i=2;i<=6;i++){static const char*w[]={"","foreground","desktop","mapping","process","owner","window"};reset();change_on_move=i;C(place_window()&&moves==1&&inputs==0&&!strcmp(g_why,w[i]));}
 {wh_plan p;memset(&p,0,sizeof p);p.count=1;p.rows[0].button='N';p.rows[0].delay=10;C(place_plan_ok(&p));p.rows[0].button='D';C(!place_plan_ok(&p));p.rows[0].button='U';C(!place_plan_ok(&p));p.rows[0].button='N';p.rows[0].dx=1;C(!place_plan_ok(&p));p.rows[0].dx=0;p.rows[0].dy=-1;C(!place_plan_ok(&p));p.rows[0].dy=0;p.count=2;C(!place_plan_ok(&p));}
 {char *a1[]={"helper","--place-window","--place-window"};char *a2[]={"helper","--place-window"};g_place=0;C(native_cli_not_called(3,a1)==2);g_place=0;C(native_cli_not_called(2,a2)==2);g_place=0;}
 printf("native guard %u checks, %u failures;zero native input\n",checks,failures);return failures?1:0;
}
