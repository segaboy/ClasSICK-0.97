/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "window-preview.h"
#include <stdlib.h>
#include <string.h>

static const wchar_t k_class[] = L"ClasSICKWindowPreviewV1";
static const wchar_t k_title[] = L"ClasSICK 0.97 - Drag the top bar | Click content | Esc: close";

static void fail(cs_win_window *w)
{
    w->failed = 1;
    if (w->window && w->live) PostMessageW(w->window, WM_CLOSE, 0, 0);
}

static void release_capture(cs_win_window *w)
{
    if (w->window && GetCapture() == w->window) {
        w->releasing = 1;
        ReleaseCapture();
        w->releasing = 0;
    }
}

static void cancel(cs_win_window *w)
{
    if (!w->live || w->failed) return;
    if (cs_window_input_cancel(&w->input) > CS_WINDOW_INPUT_CHANGED) {
        fail(w);
        return;
    }
    InvalidateRect(w->window, NULL, FALSE);
    release_capture(w);
}

static void paint(cs_win_window *w, HDC dc)
{
    RECT rc;
    int saved,ok=0,ox,oy;
    if (!GetClientRect(w->window,&rc)) { fail(w);return; }
    ox=rc.right>640?(rc.right-640)/2:0;
    oy=rc.bottom>480?(rc.bottom-480)/2:0;
    saved=SaveDC(dc);
    if (!saved) { fail(w);return; }
    /* Fixed presenter dimensions prevent its legacy integer scaling/cropping.
     * The outer client is white;the physical640x480 frame moves unscaled. */
    if (FillRect(dc,&rc,(HBRUSH)GetStockObject(WHITE_BRUSH)) &&
        SetViewportOrgEx(dc,ox,oy,NULL) &&
        IntersectClipRect(dc,0,0,640,480)!=ERROR &&
        cs_win_present(dc,&w->surface,w->scratch,(size_t)640*480u*4u,640,480)==CS_WIN_OK)
        ok=1;
    if (!RestoreDC(dc,saved)) ok=0;
    if (!ok) fail(w);else ++w->paints;
}

static void mouse(cs_win_window *w, UINT msg, WPARAM wp, LPARAM lp)
{
    RECT rc;
    LONG cw, ch, ox, oy, x, y;
    int inside, captured;
    uint32_t down;
    cs_pointer_sample s;
    if (!w->live || w->failed) return;
    if (!GetClientRect(w->window, &rc)) { fail(w); return; }
    cw = rc.right - rc.left;
    ch = rc.bottom - rc.top;
    ox = (cw > CS_WIN_WINDOW_WIDTH ? (cw - CS_WIN_WINDOW_WIDTH) / 2 : 0) + CS_WIN_WINDOW_CANVAS_X;
    oy = (ch > CS_WIN_WINDOW_HEIGHT ? (ch - CS_WIN_WINDOW_HEIGHT) / 2 : 0) + CS_WIN_WINDOW_CANVAS_Y;
    x = (LONG)(int16_t)(lp & 0xFFFF) - ox;
    y = (LONG)(int16_t)((lp >> 16) & 0xFFFF) - oy;
    inside = x >= 0 && x < CS_POINTER_WIDTH && y >= 0 && y < CS_POINTER_HEIGHT;
    captured = GetCapture() == w->window;
    down = msg == WM_LBUTTONDOWN ? 1u : msg == WM_LBUTTONUP ? 0u : (wp & MK_LBUTTON) ? 1u : 0u;
    if (!inside && !captured) {
        cancel(w);
        /* An observed release in a margin also ends held-entry suppression. */
        if (!down && !w->failed) {
            cs_pointer_sample released={0,0,0};
            if (cs_window_input_sample(&w->input,&released)>CS_WINDOW_INPUT_CHANGED) fail(w);
        }
        return;
    }
    if (down && !captured) {
        SetCapture(w->window);
        if (GetCapture() != w->window) { cancel(w); fail(w); return; }
    }
    s.dx = (int32_t)(x - w->input.pointer.x);
    s.dy = (int32_t)(y - w->input.pointer.y);
    s.buttons = down;
    if (cs_window_input_sample(&w->input, &s) > CS_WINDOW_INPUT_CHANGED) {
        fail(w);
        return;
    }
    InvalidateRect(w->window, NULL, FALSE);
    if (!down) release_capture(w);
    if (msg == WM_MOUSEMOVE && inside) {
        TRACKMOUSEEVENT t;
        memset(&t, 0, sizeof t);
        t.cbSize = sizeof t;
        t.dwFlags = TME_LEAVE;
        t.hwndTrack = w->window;
        if (!TrackMouseEvent(&t)) fail(w);
    }
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    cs_win_window *w = (cs_win_window *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (msg == WM_NCCREATE) {
        w = (cs_win_window *)((const CREATESTRUCTW *)lp)->lpCreateParams;
        SetLastError(0);
        if (!SetWindowLongPtrW(hwnd,GWLP_USERDATA,(LONG_PTR)w) && GetLastError()) return FALSE;
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
    if (w == NULL) return DefWindowProcW(hwnd, msg, wp, lp);
    switch (msg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        w->live = 0;
        w->window = NULL;
        PostQuitMessage(0);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    default:
        break;
    }
    if (!w->live) return DefWindowProcW(hwnd, msg, wp, lp);
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        if (!w->failed) paint(w, dc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_PRINTCLIENT:
        if (!w->failed) paint(w, (HDC)wp);
        return 0;
    case WM_SIZE:
        cancel(w);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        mouse(w, msg, wp, lp);
        return 0;
    case WM_MOUSELEAVE:
        if (GetCapture() != hwnd) cancel(w);
        return 0;
    case WM_CAPTURECHANGED:
        if (!w->releasing && (HWND)lp != hwnd) cancel(w);
        return 0;
    case WM_CANCELMODE:
    case WM_KILLFOCUS:
        cancel(w);
        return DefWindowProcW(hwnd, msg, wp, lp);
    case WM_SETCURSOR:
        if (LOWORD(lp) == HTCLIENT) { SetCursor(NULL); return TRUE; }
        break;
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static int register_class(HINSTANCE hi)
{
    WNDCLASSEXW wc;
    memset(&wc, 0, sizeof wc);
    wc.cbSize = sizeof wc;
    if (GetClassInfoExW(hi, k_class, &wc))
        return wc.lpfnWndProc == window_proc;
    memset(&wc, 0, sizeof wc);
    wc.cbSize = sizeof wc;
    wc.lpfnWndProc = window_proc;
    wc.hInstance = hi;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.lpszClassName = k_class;
    if (RegisterClassExW(&wc)) return 1;
    if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
        memset(&wc, 0, sizeof wc);
        wc.cbSize = sizeof wc;
        return GetClassInfoExW(hi, k_class, &wc) && wc.lpfnWndProc == window_proc;
    }
    return 0;
}

int cs_win_window_open(cs_win_window *w, int visible, uint32_t buttons)
{
    cs_window_config cfg = {80, 70, 224, 144, 18};
    cs_product_state product = {97, 0, 16, 6, 1, NULL};
    cs_cursor_token tok;
    HINSTANCE hi;
    RECT rc;
    int row;
    if (w == NULL || (visible != 0 && visible != 1) || buttons > 1u) return 0;
    memset(w, 0, sizeof *w);
    w->frame = (uint8_t *)calloc(80u * 480u, 1);
    w->scratch = (uint8_t *)calloc((size_t)640 * 480 * 4, 1);
    if (!w->frame || !w->scratch) goto bad;
    for (row = 0; row < 7; row++)
        w->bits.mask[row * 2] = w->bits.image[row * 2] = row == 3 ? 0xFE : 0x10;
    w->art.bits = &w->bits;
    w->art.width = 7; w->art.height = 7; w->art.hot_x = 3; w->art.hot_y = 3;
    if (cs_surface_init(&w->surface, w->frame, 80u * 480u, 640, 480, 80,
            CS_SURFACE_MONO1_MSB) != CS_SURFACE_OK ||
        cs_cursor_init(&w->cursor) != CS_CURSOR_OK ||
        cs_cursor_bind(&w->cursor, &w->surface, w->payload, sizeof w->payload, &tok) != CS_CURSOR_OK ||
        cs_window_input_begin(&w->input, &w->cursor, tok, &w->art, &cfg, &product, buttons) != CS_WINDOW_INPUT_OK)
        goto bad;
    hi = GetModuleHandleW(NULL);
    if (!register_class(hi)) goto bad;
    rc.left = 0; rc.top = 0; rc.right = CS_WIN_WINDOW_WIDTH; rc.bottom = CS_WIN_WINDOW_HEIGHT;
    if (!AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE)) goto bad;
    w->window = CreateWindowExW(0, k_class, k_title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, hi, w);
    if (!w->window) goto bad;
    if (visible) ShowWindow(w->window,SW_SHOW);
    w->live=1;
    if (visible) { InvalidateRect(w->window,NULL,FALSE);UpdateWindow(w->window); }
    return w->failed ? (cs_win_window_close(w), 0) : 1;
bad:
    w->failed = 1;
    cs_win_window_close(w);
    return 0;
}

void cs_win_window_close(cs_win_window *w)
{
    cs_cursor_info info;
    cs_rect old;
    if (w == NULL) return;
    if (w->window) {
        w->live = 0;
        w->releasing = 1;
        DestroyWindow(w->window);
        w->releasing = 0;
        w->window = NULL;
    }
    w->live = 0;
    if (cs_cursor_query(&w->cursor, &info) == CS_CURSOR_OK) {
        if (info.phase == CS_CURSOR_VISIBLE && cs_cursor_hide(&w->cursor, info.token) == CS_CURSOR_OK)
            info.phase = CS_CURSOR_HIDDEN;
        if (info.phase == CS_CURSOR_HIDDEN)
            cs_cursor_unbind(&w->cursor, info.token);
        else if (info.phase == CS_CURSOR_REPAIR)
            cs_cursor_abandon(&w->cursor, &old);
    }
    free(w->frame);
    free(w->scratch);
    w->frame = NULL;
    w->scratch = NULL;
}
