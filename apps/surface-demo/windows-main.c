/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "presenter.h"
#include "arena.h"
#include "../../platform/windows/input.h"
#include "../../platform/windows/clock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { FRAME_WIDTH = 513, FRAME_HEIGHT = 321, INPUT_CAPACITY = 16 };
typedef struct {
    cs_arena arena;
    cs_input_queue input;
    cs_win_clock host_clock;
    cs_clock clock, fake_source;
    cs_time deadline;
    void *backing;
    cs_surface surfaces[2];
    uint8_t *storage[2], *scratch;
    size_t scratch_size;
    unsigned active, paints;
    unsigned consumed, clock_wakeups;
    uint32_t last_source, last_sequence;
    int failed, destroyed, fake_time, flash, timer_active;
} demo_state;

static size_t scene_stride(unsigned scene)
{
    return scene == 0 ? (size_t)FRAME_WIDTH * 4u + 8u
        : ((size_t)FRAME_WIDTH + 7u) / 8u + 5u;
}

static size_t buffer_budget(void)
{
    /* Fixed original demo geometry: this is not a Macintosh RAM budget. */
    size_t bytes = (scene_stride(0) + scene_stride(1) + (size_t)FRAME_WIDTH * 4u)
        * FRAME_HEIGHT;
    size_t alignment = _Alignof(cs_input_record);
    return bytes + (alignment - bytes % alignment) % alignment
        + INPUT_CAPACITY * sizeof(cs_input_record);
}

static int init_buffers(demo_state *state, size_t capacity)
{
    cs_arena_span span;
    state->backing = malloc(capacity);
    if (state->backing == NULL || cs_arena_init(&state->arena, state->backing,
            capacity) != CS_ARENA_OK) return 0;
    for (unsigned i = 0; i < 2; ++i) {
        size_t stride = scene_stride(i), size = stride * FRAME_HEIGHT;
        if (cs_arena_alloc(&state->arena, size, 1, &span) != CS_ARENA_OK) return 0;
        state->storage[i] = span.data;
        memset(span.data, 0, span.size);
        if (cs_surface_init(&state->surfaces[i], span.data, span.size,
                FRAME_WIDTH, FRAME_HEIGHT, stride,
                i == 0 ? CS_SURFACE_RGBA8 : CS_SURFACE_MONO1_MSB) != CS_SURFACE_OK)
            return 0;
    }
    state->scratch_size = (size_t)FRAME_WIDTH * FRAME_HEIGHT * 4u;
    if (cs_arena_alloc(&state->arena, state->scratch_size, 1, &span) != CS_ARENA_OK)
        return 0;
    state->scratch = span.data;
    if (cs_arena_alloc(&state->arena, INPUT_CAPACITY * sizeof(cs_input_record),
            _Alignof(cs_input_record), &span) != CS_ARENA_OK
            || cs_input_init(&state->input, (cs_input_record *)span.data,
                INPUT_CAPACITY) != CS_INPUT_OK) return 0;
    return 1;
}

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

static int flash(demo_state *state, int on)
{
    const cs_rect strip={12,314,501,319};
    if (!fill(&state->surfaces[0],strip,on ? 0xEE6352FFu : 0xF1F4F8FFu)
            || !fill(&state->surfaces[1],strip,on ? 1u : 0u)) return 0;
    state->flash=on;
    return 1;
}

static int sample_time(HWND window, demo_state *state)
{
    cs_time now;
    int reached=0;
    if (state->fake_time) now=state->fake_source.last;
    else if (cs_win_clock_read(&state->host_clock,&now)!=CS_WIN_TIME_OK) return 0;
    if (cs_clock_observe(&state->clock,now)!=CS_TIME_OK) return 0;
    if (state->flash) {
        if (cs_time_reached(now,state->deadline,&reached)!=CS_TIME_OK) return 0;
        if (reached) {
            if (!flash(state,0)) return 0;
            InvalidateRect(window,NULL,FALSE);
        }
    }
    return 1;
}

static int consume_input(HWND window, demo_state *state)
{
    cs_input_record record;
    cs_input_result result;
    while ((result = cs_input_pop(&state->input, &record)) == CS_INPUT_OK) {
        ++state->consumed;
        state->last_source = record.event.source;
        state->last_sequence = record.sequence;
        if (record.event.action != CS_KEY_PRESS || record.event.repeat != 0) continue;
        if (record.event.key == CS_KEY_SPACE) {
            if (cs_time_add(state->clock.last,(cs_time){0,250000000},&state->deadline)
                    !=CS_TIME_OK || !flash(state,1)) return 0;
            state->active ^= 1u;
            title(window, state->active);
            InvalidateRect(window, NULL, FALSE);
        } else if (record.event.key == CS_KEY_ESCAPE) {
            DestroyWindow(window);
            return 1;
        }
    }
    return result == CS_INPUT_ERR_EMPTY;
}

static int submit_input(HWND window, demo_state *state, const cs_input_event *event)
{
    if (!sample_time(window,state) || cs_input_push(&state->input, event) != CS_INPUT_OK
            || !consume_input(window, state)) {
        state->failed = 1;
        return 0;
    }
    return 1;
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
    case WM_TIMER:
        if (wparam!=1 || !state->timer_active || state->destroyed) return 0;
        ++state->clock_wakeups;
        if (!sample_time(window,state)) {
            state->failed=1;
            PostMessageW(window,WM_CLOSE,0,0);
        }
        return 0;
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
    case WM_KEYUP: {
        cs_input_event event;
        int mapped = cs_win_key_event(message, wparam, lparam, 2, &event);
        if (mapped == CS_WIN_INPUT_MAPPED) {
            if (!submit_input(window, state, &event)) PostMessageW(window, WM_CLOSE, 0, 0);
            return 0;
        }
        break;
    }
    case WM_DESTROY:
        if (state->timer_active) { KillTimer(window,1); state->timer_active=0; }
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
            (state->active == 0 ? RGB(30,40,58) : RGB(0,0,0))
        && GetPixel(dc,12*scale,316*scale)==(state->active==0
            ? (state->flash ? RGB(238,99,82) : RGB(241,244,248))
            : (state->flash ? RGB(0,0,0) : RGB(255,255,255)));
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

static int verify_input(HWND window, demo_state *state)
{
    const cs_input_event press = {1,CS_KEY_SPACE,CS_KEY_PRESS,0};
    const cs_input_event release = {1,CS_KEY_SPACE,CS_KEY_RELEASE,0};
    const cs_input_event repeat = {1,CS_KEY_SPACE,CS_KEY_PRESS,1};
    const cs_input_event escape = {1,CS_KEY_ESCAPE,CS_KEY_PRESS,0};
    cs_input_record before[INPUT_CAPACITY];
    if (!submit_input(window, state, &press) || state->active != 1
            || !submit_input(window, state, &release)
            || !submit_input(window, state, &repeat) || state->active != 1) return 0;
    SendMessageW(window, WM_KEYUP, VK_SPACE, (LPARAM)INT32_MIN);
    SendMessageW(window, WM_KEYDOWN, VK_SPACE, 0);
    if (state->active != 0 || state->consumed != 5 || state->last_source != 2
            || state->last_sequence != 5 || state->input.count != 0) return 0;
    SendMessageW(window, WM_KEYDOWN, 'A', 0);
    if (state->consumed != 5 || !verify_print(window, state, 1)) return 0;
    if (cs_input_reset(&state->input) != CS_INPUT_OK) return 0;
    for (unsigned i = 0; i < INPUT_CAPACITY; ++i)
        if (cs_input_push(&state->input, &repeat) != CS_INPUT_OK) return 0;
    memcpy(before, state->input.storage, sizeof(before));
    if (submit_input(window, state, &press) || !state->failed
            || state->input.count != INPUT_CAPACITY || state->input.next_sequence != 17
            || memcmp(before, state->input.storage, sizeof(before)) != 0) return 0;
    state->failed = 0;
    if (!consume_input(window, state) || state->input.count != 0 || state->active != 0
            || state->last_sequence != 16 || state->last_source != 1) return 0;
    if (!submit_input(window, state, &escape) || !state->destroyed || IsWindow(window)) return 0;
    puts("SPEC-0004 hidden viewer synthetic/native FIFO, repeat, overflow/recovery, close: PASS");
    return 1;
}

static int verify_clock(HWND window,demo_state *state)
{
    const cs_input_event press={1,CS_KEY_SPACE,CS_KEY_PRESS,0};
    const cs_input_event repeat={1,CS_KEY_SPACE,CS_KEY_PRESS,1};
    if (!submit_input(window,state,&press) || !state->flash || !verify_print(window,state,1)) return 0;
    if (cs_clock_advance(&state->fake_source,(cs_time){0,249999999})!=CS_TIME_OK) return 0;
    SendMessageW(window,WM_TIMER,1,0);
    if (!state->flash || !verify_print(window,state,1) || !submit_input(window,state,&repeat)
            || state->deadline.seconds!=0 || state->deadline.nanoseconds!=250000000) return 0;
    if (cs_clock_advance(&state->fake_source,(cs_time){0,1})!=CS_TIME_OK) return 0;
    SendMessageW(window,WM_TIMER,1,0);
    if (state->flash || !verify_print(window,state,1)) return 0;
    SendMessageW(window,WM_KEYDOWN,VK_SPACE,0);
    if (!state->flash || state->active!=0 || !verify_print(window,state,1)) return 0;
    if (cs_clock_advance(&state->fake_source,(cs_time){7,0})!=CS_TIME_OK) return 0;
    SendMessageW(window,WM_TIMER,1,0);
    if (state->flash || !verify_print(window,state,1) || state->failed) return 0;
    SendMessageW(window,WM_CLOSE,0,0);
    if (!state->destroyed || state->timer_active || IsWindow(window)) return 0;
    puts("SPEC-0005 hidden viewer fake/native-input deadline pixels and timer lifecycle: PASS");
    return 1;
}

static int verify_live_clock(HWND window,demo_state *state)
{
    MSG message;
    SendMessageW(window,WM_KEYDOWN,VK_SPACE,0);
    while (state->clock_wakeups==0) {
        BOOL received=GetMessageW(&message,NULL,0,0);
        if (received<=0) return 0;
        TranslateMessage(&message); DispatchMessageW(&message);
    }
    if (state->failed || (state->clock.last.seconds==0 && state->clock.last.nanoseconds==0)
            || !verify_print(window,state,1)) return 0;
    SendMessageW(window,WM_CLOSE,0,0);
    if (!state->destroyed || state->timer_active || IsWindow(window)) return 0;
    puts("SPEC-0005 hidden viewer live QPC and delivered Windows timer: PASS");
    return 1;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command, int show)
{
    WNDCLASSW klass = {0};
    HWND window = NULL;
    RECT rectangle = {0,0,FRAME_WIDTH,FRAME_HEIGHT};
    demo_state state = {0};
    int result = 1, registered = 0;
    int verify = strcmp(command, "--verify") == 0;
    int verify_memory = strcmp(command, "--verify-memory") == 0;
    int verify_keys = strcmp(command, "--verify-input") == 0;
    int verify_key_memory = strcmp(command, "--verify-input-memory") == 0;
    int verify_time=strcmp(command,"--verify-clock")==0;
    int verify_live=strcmp(command,"--verify-clock-live")==0;
    (void)previous;
    if (command[0] != '\0' && !verify && !verify_memory && !verify_keys && !verify_key_memory && !verify_time && !verify_live) return 2;
    if (verify_key_memory) {
        if (buffer_budget() != 1342744u || init_buffers(&state, 1342743u)
                || state.backing == NULL || state.arena.used != 1342422u
                || state.scratch == NULL || state.input.storage != NULL) goto cleanup;
        puts("SPEC-0004 viewer one-byte-short input storage: PASS");
        result = 0;
        goto cleanup;
    }
    if (verify_memory) {
        /* Independent literal lengths for the fixed color/mono/scratch scene. */
        if (buffer_budget() != 1342744u || init_buffers(&state, 1342421u)
                || state.backing == NULL || state.arena.used != 683730u
                || state.scratch != NULL) goto cleanup;
        puts("SPEC-0003 viewer undersized backing pool: PASS");
        result = 0;
        goto cleanup;
    }
    /* Set before any UI: keep frame pixels sharp at non-integer desktop scaling. */
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
        goto cleanup;
    state.fake_time=verify_time;
    if (cs_clock_init(&state.clock,(cs_time){0,0})!=CS_TIME_OK
            || cs_clock_init(&state.fake_source,(cs_time){0,0})!=CS_TIME_OK
            || (!state.fake_time && cs_win_clock_init(&state.host_clock)!=CS_WIN_TIME_OK)) goto cleanup;
    if (!init_buffers(&state, buffer_budget()) || !scenes(&state)) goto cleanup;
    klass.lpfnWndProc = window_proc; klass.hInstance = instance;
    klass.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    if (klass.hCursor == NULL) goto cleanup;
    klass.lpszClassName = L"ClasSICK097OriginalSurfaceViewer";
    if (RegisterClassW(&klass) == 0) goto cleanup;
    registered = 1;
    if (!verify && !verify_keys && !verify_time && !verify_live) { rectangle.right *= 2; rectangle.bottom *= 2; }
    if (!AdjustWindowRect(&rectangle, WS_OVERLAPPEDWINDOW, FALSE)) goto cleanup;
    window = CreateWindowExW(0, klass.lpszClassName, L"ClasSICK 0.97",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        rectangle.right - rectangle.left, rectangle.bottom - rectangle.top,
        NULL, NULL, instance, &state);
    if (window == NULL || !resize_client(window, verify || verify_keys || verify_time || verify_live ? 1 : 2)) goto cleanup;
    title(window, 0);
    if (verify_time || verify_live || (!verify && !verify_keys)) {
        if (SetTimer(window,1,25,NULL)==0) goto cleanup;
        state.timer_active=1;
    }
    if (verify_live) {
        if (!verify_live_clock(window,&state)) goto cleanup;
        result=0;
    } else if (verify_time) {
        if (!verify_clock(window,&state)) goto cleanup;
        result=0;
    } else if (verify_keys) {
        if (!verify_input(window, &state)) goto cleanup;
        result = 0;
    } else if (verify) {
        if (state.arena.capacity != 1342744u || state.arena.used != 1342744u
                || state.storage[0] != state.arena.storage
                || state.storage[1] != state.arena.storage + 661260u
                || state.scratch != state.arena.storage + 683730u
                || (unsigned char *)state.input.storage != state.arena.storage + 1342424u)
            goto cleanup;
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
    /* Window callbacks have ended; every span is retired before pool release. */
    (void)cs_input_reset(&state.input);
    (void)cs_arena_reset(&state.arena);
    free(state.backing);
    if (result != 0) fputs("ClasSICK 0.97 viewer failed; see SPEC-0002.\n", stderr);
    return result;
}
