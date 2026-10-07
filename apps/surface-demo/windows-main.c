/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "presenter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { FRAME_WIDTH = 513, FRAME_HEIGHT = 321 };
typedef struct {
    cs_surface surfaces[2];
    uint8_t *storage[2], *scratch;
    size_t scratch_size;
    unsigned active, paints;
    int failed, destroyed;
} demo_state;

static int fill(cs_surface *surface, cs_rect rect, uint32_t color)
{
    return cs_surface_fill(surface, rect, color) == CS_SURFACE_OK;
}

static int scenes(demo_state *state)
{
    static const uint32_t colors[6] = {
        0xEE6352FFu, 0xF5B841FFu, 0x60B894FFu, 0x4597D2FFu, 0x8175BAFFu, 0xD36B9CFFu
    };
    cs_surface *color = &state->surfaces[0], *mono = &state->surfaces[1];
    if (cs_surface_clear(color, 0xF1F4F8FFu) != CS_SURFACE_OK
            || cs_surface_clear(mono, 0) != CS_SURFACE_OK) return 0;
    if (!fill(color, (cs_rect){12,12,501,57}, 0x1E283AFFu)
            || !fill(color, (cs_rect){26,27,37,42}, 0x60B894FFu)
            || !fill(color, (cs_rect){44,27,55,42}, 0xF5B841FFu)
            || !fill(color, (cs_rect){62,27,73,42}, 0xEE6352FFu)
            || !fill(color, (cs_rect){92,30,350,38}, 0xF1F4F8FFu)
            || !fill(color, (cs_rect){12,69,188,253}, 0xDCE3EBFFu)
            || !fill(color, (cs_rect){200,69,501,253}, 0xDCE3EBFFu)) return 0;
    for (int32_t y = 0; y < 5; ++y) for (int32_t x = 0; x < 5; ++x) {
        if (!fill(color, (cs_rect){24+x*31,81+y*31,49+x*31,106+y*31},
                colors[(x+y)%6])) return 0;
    }
    if (!fill(color, (cs_rect){221,90,395,219}, 0x4597D200u)
            || !fill(color, (cs_rect){267,123,468,234}, 0xEE635280u)
            || !fill(color, (cs_rect){301,153,426,203}, 0xF5B841FFu)) return 0;
    for (int32_t x = 0; x < 6; ++x)
        if (!fill(color, (cs_rect){12+x*82,265,94+x*82,309}, colors[x])) return 0;
    if (!fill(mono, (cs_rect){12,12,501,309}, 1)
            || !fill(mono, (cs_rect){14,14,499,307}, 0)
            || !fill(mono, (cs_rect){26,27,487,55}, 1)) return 0;
    for (int32_t y = 0; y < 10; ++y) for (int32_t x = 0; x < 12; ++x)
        if ((x+y)%2 == 0 && !fill(mono,
                (cs_rect){26+x*16,73+y*16,42+x*16,89+y*16}, 1)) return 0;
    for (int32_t x = 0; x < 8; ++x)
        if (!fill(mono, (cs_rect){241+x*31,73,250+x*31,278-x*15}, 1)) return 0;
    /* Original edge bars deliberately cross the right edge of the logical frame. */
    return fill(color, (cs_rect){485,100,540,107}, 0x1E283AFFu)
        && fill(mono, (cs_rect){485,290,540,297}, 1);
}

static void title(HWND window, unsigned active)
{
    SetWindowTextW(window, active == 0
        ? L"ClasSICK 0.97 - Color | Space: switch view | Esc: close"
        : L"ClasSICK 0.97 - Monochrome | Space: switch view | Esc: close");
}

static void paint(HWND window, HDC dc, demo_state *state)
{
    RECT client;
    if (!GetClientRect(window, &client)
            || cs_win_present(dc, &state->surfaces[state->active], state->scratch,
                state->scratch_size, client.right, client.bottom) != CS_WIN_OK) {
        state->failed = 1;
        PostMessageW(window, WM_CLOSE, 0, 0);
    }
    ++state->paints;
}

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    demo_state *state = (demo_state *)GetWindowLongPtrW(window, GWLP_USERDATA);
    if (message == WM_NCCREATE) {
        const CREATESTRUCTW *create = (const CREATESTRUCTW *)lparam;
        state = (demo_state *)create->lpCreateParams;
        SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR)state);
        return TRUE;
    }
    if (state == NULL) return DefWindowProcW(window, message, wparam, lparam);
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT ps = {0};
        HDC dc = BeginPaint(window, &ps);
        paint(window, dc, state);
        EndPaint(window, &ps);
        return 0;
    }
    case WM_PRINTCLIENT:
        paint(window, (HDC)wparam, state);
        return 0;
    case WM_ERASEBKGND: return 1;
    case WM_SIZE:
        InvalidateRect(window, NULL, FALSE);
        return 0;
    case WM_DPICHANGED: {
        const RECT *suggested = (const RECT *)lparam;
        if (!SetWindowPos(window, NULL, suggested->left, suggested->top,
                suggested->right - suggested->left, suggested->bottom - suggested->top,
                SWP_NOZORDER|SWP_NOACTIVATE)) state->failed = 1;
        return 0;
    }
    case WM_KEYDOWN:
        if (wparam == VK_SPACE && (lparam & ((LPARAM)1 << 30)) == 0) {
            state->active ^= 1u;
            title(window, state->active);
            InvalidateRect(window, NULL, FALSE);
        } else if (wparam == VK_ESCAPE) DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        state->destroyed = 1;
        PostQuitMessage(0);
        return 0;
    case WM_NCDESTROY:
        SetWindowLongPtrW(window, GWLP_USERDATA, 0);
        break;
    default: break;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

static int verify_print(HWND window, demo_state *state, int scale)
{
    BITMAPINFO info = {0};
    RECT client;
    HDC dc = CreateCompatibleDC(NULL);
    void *bytes = NULL;
    HBITMAP bitmap = NULL, previous = NULL;
    int passed = 0;
    if (dc == NULL || !GetClientRect(window, &client)
            || client.right != FRAME_WIDTH * scale || client.bottom != FRAME_HEIGHT * scale)
        goto cleanup;
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = client.right; info.bmiHeader.biHeight = -client.bottom;
    info.bmiHeader.biPlanes = 1; info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bytes, NULL, 0);
    if (bitmap == NULL || bytes == NULL) goto cleanup;
    previous = (HBITMAP)SelectObject(dc, bitmap);
    if (previous == NULL || previous == (HBITMAP)HGDI_ERROR) goto cleanup;
    SendMessageW(window, WM_PRINTCLIENT, (WPARAM)dc, PRF_CLIENT);
    passed = !state->failed && state->paints != 0
        && GetPixel(dc, 2 * scale, 2 * scale) ==
            (state->active == 0 ? RGB(241,244,248) : RGB(255,255,255))
        && GetPixel(dc, 12 * scale, 12 * scale) ==
            (state->active == 0 ? RGB(30,40,58) : RGB(0,0,0));
cleanup:
    if (previous != NULL && previous != (HBITMAP)HGDI_ERROR) SelectObject(dc, previous);
    if (bitmap != NULL) DeleteObject(bitmap);
    if (dc != NULL) DeleteDC(dc);
    return passed;
}

static int resize_client(HWND window, int scale)
{
    RECT rect = {0,0,FRAME_WIDTH*scale,FRAME_HEIGHT*scale};
    UINT dpi = GetDpiForWindow(window);
    return dpi != 0 && AdjustWindowRectExForDpi(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0, dpi)
        && SetWindowPos(window, NULL, 0, 0, rect.right - rect.left,
            rect.bottom - rect.top, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command, int show)
{
    WNDCLASSW klass = {0};
    HWND window = NULL;
    RECT rectangle = {0,0,FRAME_WIDTH,FRAME_HEIGHT};
    demo_state state = {0};
    int result = 1, registered = 0;
    int verify = strcmp(command, "--verify") == 0;
    (void)previous;
    if (command[0] != '\0' && !verify) return 2;
    /* Set before any UI: keep frame pixels sharp at non-integer desktop scaling. */
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
        goto cleanup;
    state.scratch_size = (size_t)FRAME_WIDTH * FRAME_HEIGHT * 4u;
    state.scratch = malloc(state.scratch_size);
    for (unsigned i = 0; i < 2; ++i) {
        size_t stride = i == 0 ? (size_t)FRAME_WIDTH * 4u + 8u
            : ((size_t)FRAME_WIDTH + 7u) / 8u + 5u;
        size_t size = stride * FRAME_HEIGHT;
        state.storage[i] = calloc(size, 1);
        if (state.storage[i] == NULL || cs_surface_init(&state.surfaces[i],
                state.storage[i], size, FRAME_WIDTH, FRAME_HEIGHT, stride,
                i == 0 ? CS_SURFACE_RGBA8 : CS_SURFACE_MONO1_MSB) != CS_SURFACE_OK)
            goto cleanup;
    }
    if (state.scratch == NULL || !scenes(&state)) goto cleanup;
    klass.lpfnWndProc = window_proc; klass.hInstance = instance;
    klass.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    if (klass.hCursor == NULL) goto cleanup;
    klass.lpszClassName = L"ClasSICK097OriginalSurfaceViewer";
    if (RegisterClassW(&klass) == 0) goto cleanup;
    registered = 1;
    if (!verify) { rectangle.right *= 2; rectangle.bottom *= 2; }
    if (!AdjustWindowRect(&rectangle, WS_OVERLAPPEDWINDOW, FALSE)) goto cleanup;
    window = CreateWindowExW(0, klass.lpszClassName, L"ClasSICK 0.97",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        rectangle.right - rectangle.left, rectangle.bottom - rectangle.top,
        NULL, NULL, instance, &state);
    if (window == NULL || !resize_client(window, verify ? 1 : 2)) goto cleanup;
    title(window, 0);
    if (verify) {
        if (!verify_print(window, &state, 1)) goto cleanup;
        if (GetAwarenessFromDpiAwarenessContext(GetThreadDpiAwarenessContext())
                    != DPI_AWARENESS_PER_MONITOR_AWARE
                || !resize_client(window, 2)
                || !verify_print(window, &state, 2)) goto cleanup;
        SendMessageW(window, WM_KEYDOWN, VK_SPACE, 0);
        if (state.active != 1 || !verify_print(window, &state, 2)) goto cleanup;
        SendMessageW(window, WM_KEYDOWN, VK_SPACE, (LPARAM)1 << 30);
        if (state.active != 1) goto cleanup;
        SendMessageW(window, WM_CLOSE, 0, 0);
        if (!state.destroyed || IsWindow(window)) goto cleanup;
        puts("SPEC-0002 hidden window create/resize/paint/switch/close: PASS");
        result = 0;
    } else {
        MSG message;
        ShowWindow(window, show); UpdateWindow(window);
        for (;;) {
            BOOL received = GetMessageW(&message, NULL, 0, 0);
            if (received == -1) goto cleanup;
            if (received == 0) break;
            TranslateMessage(&message); DispatchMessageW(&message);
        }
        result = state.failed ? 1 : 0;
    }
cleanup:
    if (window != NULL && IsWindow(window)) DestroyWindow(window);
    if (registered) UnregisterClassW(klass.lpszClassName, instance);
    free(state.storage[0]); free(state.storage[1]); free(state.scratch);
    if (result != 0) fputs("ClasSICK 0.97 viewer failed; see SPEC-0002.\n", stderr);
    return result;
}
