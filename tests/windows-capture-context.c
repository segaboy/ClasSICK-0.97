/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 Dean Howell */
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"
static int mode, owner_calls, reads;
DWORD GetWindowThreadProcessId(HWND h,DWORD *p) { if (h==(HWND)2) { *p=mode==7?99:42; return 6; } owner_calls++; if(mode==2) return 0; *p=mode==3?99:42; if(mode==4 && owner_calls==2) *p=99; return 5; }
BOOL GetCursorPos(POINT *p){reads++;p->x=12;p->y=34;return mode!=5;}
BOOL GetPhysicalCursorPos(POINT *p){p->x=24;p->y=68;return mode!=5;}
BOOL GetClipCursor(RECT *p){p->left=1;p->top=2;p->right=3;p->bottom=4;return mode!=5;}
BOOL GetCursorInfo(CURSORINFO *p){if(p->cbSize!=sizeof(*p))abort();p->flags=CURSOR_SHOWING;return mode!=5;}
void *GetWindowDpiAwarenessContext(HWND h){(void)h;return mode==5?NULL:(void *)1;}
BOOL GetGUIThreadInfo(DWORD t,GUITHREADINFO *p){if(t!=5||p->cbSize!=sizeof(*p))abort();p->hwndCapture=mode==6?NULL:(HWND)2;return mode!=5;}
#include "../tools/windows-input/context.c"
int main(int argc,char **argv) { int result; if(argc!=2)return 9;mode=atoi(argv[1]); result=wih_context_snapshot(mode==1?NULL:(HWND)1,42,stdout); if((mode>=1&&mode<=4) ? result!=1 : result!=0)return 10; if(mode>=1&&mode<=3&&reads!=0)return 11; return 0; }
