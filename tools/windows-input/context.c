/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Prereqs: Windows 10 1607+ SDK, C99, link user32. Read-only observations only.
   Caller verifies owned process, image, window and input desktop.
   Refusal codes: 0 none, 1 invalid input, 2 owner query failed, 3 PID mismatch,
   4 window thread/PID changed or failed on recheck, 5 internal format overflow.
   Return: 0 snapshot emitted, 1 refusal emitted, -1 bad stream or write error. */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#ifndef WINVER
#define WINVER 0x0A00
#endif
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

typedef struct { char b[1536]; size_t n; int bad; } wih_buf;

static void wih_add(wih_buf *o, const char *fmt, ...)
{
    va_list ap;
    int r;
    if (o->bad) return;
    va_start(ap, fmt);
    r = vsnprintf(o->b + o->n, sizeof(o->b) - o->n, fmt, ap);
    va_end(ap);
    if (r < 0 || (size_t)r >= sizeof(o->b) - o->n) o->bad = 1;
    else o->n += (size_t)r;
}

static void wih_pt(wih_buf *o, const char *name, BOOL ok, POINT p)
{
    if (ok)
        wih_add(o, "\"%s\":{\"ok\":true,\"x\":%ld,\"y\":%ld},", name, (long)p.x, (long)p.y);
    else
        wih_add(o, "\"%s\":{\"ok\":false,\"x\":null,\"y\":null},", name);
}

static void wih_gather(wih_buf *o, HWND window, DWORD tid, DWORD expected_pid)
{
    POINT p;
    RECT rc;
    CURSORINFO ci;
    GUITHREADINFO gi;
    BOOL ok;

    p.x = 0; p.y = 0;
    ok = GetCursorPos(&p);
    wih_pt(o, "cursor_screen", ok, p);
    p.x = 0; p.y = 0;
    ok = GetPhysicalCursorPos(&p);
    wih_pt(o, "cursor_physical", ok, p);

    rc.left = rc.top = rc.right = rc.bottom = 0;
    ok = GetClipCursor(&rc);
    if (ok)
        wih_add(o, "\"clip_rect\":{\"ok\":true,\"left\":%ld,\"top\":%ld,\"right\":%ld,\"bottom\":%ld},",
                (long)rc.left, (long)rc.top, (long)rc.right, (long)rc.bottom);
    else
        wih_add(o, "\"clip_rect\":{\"ok\":false,\"left\":null,\"top\":null,\"right\":null,\"bottom\":null},");

    ZeroMemory(&ci, sizeof(ci));
    ci.cbSize = (DWORD)sizeof(ci);
    ok = (ci.cbSize == sizeof(CURSORINFO)) ? GetCursorInfo(&ci) : FALSE;
    if (ok)
        wih_add(o, "\"cursor_info\":{\"ok\":true,\"flags\":%lu,\"showing\":%s},",
                (unsigned long)ci.flags, (ci.flags & CURSOR_SHOWING) ? "true" : "false");
    else
        wih_add(o, "\"cursor_info\":{\"ok\":false,\"flags\":null,\"showing\":null},");

    /* Opaque context: only presence is reported; it is never dereferenced or interpreted. */
    wih_add(o, "\"dpi_context_present\":%s,",
            GetWindowDpiAwarenessContext(window) != NULL ? "true" : "false");

    ZeroMemory(&gi, sizeof(gi));
    gi.cbSize = (DWORD)sizeof(gi);
    ok = (gi.cbSize == sizeof(GUITHREADINFO)) ? GetGUIThreadInfo(tid, &gi) : FALSE;
    if (!ok) {
        wih_add(o, "\"gui_capture\":{\"query_ok\":false,\"capture_present\":null,\"capture_pid\":null,\"pid_match\":null}");
    } else if (gi.hwndCapture == NULL) {
        wih_add(o, "\"gui_capture\":{\"query_ok\":true,\"capture_present\":false,\"capture_pid\":null,\"pid_match\":null}");
    } else {
        DWORD cpid = 0;
        DWORD ctid = GetWindowThreadProcessId(gi.hwndCapture, &cpid);
        if (ctid != 0 && cpid != 0)
            wih_add(o, "\"gui_capture\":{\"query_ok\":true,\"capture_present\":true,\"capture_pid\":%lu,\"pid_match\":%s}",
                    (unsigned long)cpid, cpid == expected_pid ? "true" : "false");
        else
            wih_add(o, "\"gui_capture\":{\"query_ok\":true,\"capture_present\":true,\"capture_pid\":null,\"pid_match\":null}");
    }
}

int wih_context_snapshot(HWND window, DWORD expected_pid, FILE *out)
{
    static const char nulls[] =
        "\"cursor_screen\":null,\"cursor_physical\":null,\"clip_rect\":null,"
        "\"cursor_info\":null,\"dpi_context_present\":null,\"gui_capture\":null";
    wih_buf o;
    DWORD tid = 0, pid = 0, tid2, pid2 = 0;
    int code = 0;
    char wp[16], rcs[8];

    if (out == NULL) return -1;
    o.n = 0; o.bad = 0; o.b[0] = '\0';

    if (window == NULL || expected_pid == 0) code = 1;
    else {
        tid = GetWindowThreadProcessId(window, &pid);
        if (tid == 0 || pid == 0) code = 2;
        else if (pid != expected_pid) code = 3;
    }
    if (code == 0) {
        wih_gather(&o, window, tid, expected_pid);
        tid2 = GetWindowThreadProcessId(window, &pid2);
        if (tid2 == 0 || pid2 == 0 || tid2 != tid || pid2 != pid) code = 4;
        else if (o.bad) code = 5;
    }

    if (code == 0) {
        snprintf(wp, sizeof(wp), "%lu", (unsigned long)pid);
        snprintf(rcs, sizeof(rcs), "null");
    } else {
        snprintf(wp, sizeof(wp), "null");
        snprintf(rcs, sizeof(rcs), "%d", code);
    }

    if (fprintf(out, "{\"schema\":2,\"expected_pid\":%lu,\"window_pid\":%s,"
                "\"pid_match\":%s,\"refused\":%s,\"refusal_code\":%s,%s,"
                "\"observation_only\":true}\n",
                (unsigned long)expected_pid, wp,
                code == 0 ? "true" : (code == 3 ? "false" : "null"),
                code == 0 ? "false" : "true", rcs,
                code == 0 ? o.b : nulls) < 0)
        return -1;
    if (fflush(out) != 0 || ferror(out)) return -1;
    return code == 0 ? 0 : 1;
}
