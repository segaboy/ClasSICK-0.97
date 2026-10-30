/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 Dean Howell */
#ifndef FAKE_WINDOWS_H
#define FAKE_WINDOWS_H
#include <string.h>
typedef void *HWND; typedef unsigned long DWORD; typedef int BOOL;
typedef struct { long x,y; } POINT;
typedef struct { long left,top,right,bottom; } RECT;
typedef struct { DWORD cbSize,flags; void *hCursor; POINT ptScreenPos; } CURSORINFO;
typedef struct { DWORD cbSize,flags; HWND hwndActive,hwndFocus,hwndCapture,hwndMenuOwner,hwndMoveSize,hwndCaret; RECT rcCaret; } GUITHREADINFO;
#define FALSE 0
#define TRUE 1
#define CURSOR_SHOWING 1u
#define ZeroMemory(a,b) memset((a),0,(b))
DWORD GetWindowThreadProcessId(HWND,DWORD *); BOOL GetCursorPos(POINT *); BOOL GetPhysicalCursorPos(POINT *); BOOL GetClipCursor(RECT *); BOOL GetCursorInfo(CURSORINFO *); void *GetWindowDpiAwarenessContext(HWND); BOOL GetGUIThreadInfo(DWORD,GUITHREADINFO *);
#endif
