/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 Dean Howell
 *
 * Development tool, not OS code. Windows 11 adapter for helper.c.
 *
 * test-targets.h (caller-generated, kept OUTSIDE Git, found via -I). It is included after
 * wih_target is defined here, and must contain exactly:
 *   static const wih_target wih_targets[2] = {
 *     { L"snow", L"C:\\abs\\path\\snow.exe", { 32 SHA-256 bytes }, PID_DWORD, CREATION_FILETIME_U64 },
 *     { L"vbox", L"C:\\abs\\path\\vbox.exe", { 32 SHA-256 bytes }, PID_DWORD, CREATION_FILETIME_U64 }
 *   };
 * Index 0 is role snow, index 1 is role vbox. PID 0 means inactive (live refused).
 * CREATION_FILETIME_U64 = (dwHighDateTime << 32) | dwLowDateTime. */
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <bcrypt.h>
#include "helper.h"
#include "capture.h"

/* Test seam: define WH_CAPTURE_QUERY(hwnd,pid,out) to substitute a pure result. */
#ifndef WH_CAPTURE_QUERY
#define WH_CAPTURE_QUERY(h, p, o) wh_capture_query((h), (p), (o))
#endif
static wh_capture g_cap = { 0, "not queried", 0 };

typedef struct {
  const wchar_t *role, *path;
  unsigned char sha256[32];
  DWORD pid;
  unsigned long long created;
} wih_target;
#include "test-targets.h"

static HANDLE g_proc, g_file;
static const wih_target *g_t;
static HWND g_hwnd;
static int g_swap;
static ULONGLONG g_t0;

typedef struct { DWORD pid; int n; HWND h; } EW;

static unsigned long long ft(const FILETIME *f) {
  return ((unsigned long long)f->dwHighDateTime << 32) | f->dwLowDateTime;
}

static int alive(void) {
  FILETIME c, x, k, u;
  return WaitForSingleObject(g_proc, 0) == WAIT_TIMEOUT &&
         GetProcessTimes(g_proc, &c, &x, &k, &u) && ft(&c) == g_t->created;
}

static BOOL CALLBACK ew(HWND h, LPARAM l) {
  EW *e = (EW *)l;
  DWORD p = 0;
  GetWindowThreadProcessId(h, &p);
  if (p == e->pid && IsWindowVisible(h)) {
    RECT rc;
    int has = GetClientRect(h, &rc);
    /* A zero-area utility cannot satisfy the client-area event guard. */
    if (has && rc.right > rc.left && rc.bottom > rc.top) { e->n++; e->h = h; }
    printf("{\"owned_window\":{\"handle\":\"%p\",\"client_read\":%s,\"width\":%ld,\"height\":%ld}}\n",
           (void *)h, has ? "true" : "false", has ? (long)(rc.right-rc.left) : 0L,
           has ? (long)(rc.bottom-rc.top) : 0L);
  }
  return TRUE;
}

static int fg_ok(void) {
  HWND f = GetForegroundWindow();
  return f && GetAncestor(f, GA_ROOT) == g_hwnd;
}

static int hash_ok(void) {
  BCRYPT_ALG_HANDLE a = NULL;
  BCRYPT_HASH_HANDLE h = NULL;
  unsigned char buf[65536], d[32];
  DWORD n;
  int ok = 0, good = 1;
  g_file = CreateFileW(g_t->path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                       FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  if (g_file == INVALID_HANDLE_VALUE) { g_file = NULL; return 0; }
  if (BCryptOpenAlgorithmProvider(&a, BCRYPT_SHA256_ALGORITHM, NULL, 0) < 0) return 0;
  if (BCryptCreateHash(a, &h, NULL, 0, NULL, 0, 0) >= 0) {
    for (;;) {
      if (!ReadFile(g_file, buf, sizeof buf, &n, NULL)) { good = 0; break; }
      if (!n) break;
      if (BCryptHashData(h, buf, n, 0) < 0) { good = 0; break; }
    }
    ok = good && BCryptFinishHash(h, d, 32, 0) >= 0 && !memcmp(d, g_t->sha256, 32);
    BCryptDestroyHash(h);
  }
  BCryptCloseAlgorithmProvider(a, 0);
  return ok;
}

static const char *g_why = "none";
static int g_swap_known;

/* Read-only: the input desktop must be named Default. */
static int desktop_ok(void) {
  wchar_t nm[32];
  DWORD need = 0;
  HDESK d = OpenInputDesktop(0, FALSE, DESKTOP_READOBJECTS);
  int ok;
  if (!d) return 0;
  ok = GetUserObjectInformationW(d, UOI_NAME, nm, sizeof nm, &need) && !wcscmp(nm, L"Default");
  CloseDesktop(d);
  return ok;
}

static int deny(const char *why) { g_why = why; return 0; }

static int base_ok(void) {
  DWORD q = 0;
  if (!alive()) return deny("process");
  if (!IsWindow(g_hwnd) || !IsWindowVisible(g_hwnd)) return deny("window");
  GetWindowThreadProcessId(g_hwnd, &q);
  if (q != g_t->pid) return deny("owner");
  if (!fg_ok()) return deny("foreground");
  if ((GetSystemMetrics(SM_SWAPBUTTON) != 0) != g_swap) return deny("mapping");
  if (!desktop_ok()) return deny("desktop");
  return 1;
}

static int guard(void *c) {
  DWORD p = 0;
  int phys;
  POINT pt;
  RECT rc;
  HWND w;
  (void)c;
  g_cap.verified=0;g_cap.stage="not queried";g_cap.hr=0;
  if (!alive()) return deny("process");
  if (!IsWindow(g_hwnd) || !IsWindowVisible(g_hwnd)) return deny("window");
  GetWindowThreadProcessId(g_hwnd, &p);
  if (p != g_t->pid) return deny("owner");
  if (!fg_ok()) return deny("foreground");
  if ((GetSystemMetrics(SM_SWAPBUTTON) != 0) != g_swap) return deny("mapping");
  if (!desktop_ok()) return deny("desktop");
  if (!GetCursorPos(&pt)) return deny("cursor");
  w = WindowFromPoint(pt);
  phys = w && GetAncestor(w, GA_ROOT) == g_hwnd &&
         ScreenToClient(g_hwnd, &pt) && GetClientRect(g_hwnd, &rc) &&
         pt.x >= rc.left && pt.x < rc.right && pt.y >= rc.top && pt.y < rc.bottom;
  if (!phys) {
    /* Capture metadata is read-only receipt data; it never grants input. */
    memset(&g_cap, 0, sizeof g_cap);
    WH_CAPTURE_QUERY((void *)g_hwnd, (unsigned long)g_t->pid, &g_cap);
    if (!base_ok()) return 0;
    return deny("client area");
  }
  printf("{\"guard_client\":[%ld,%ld],\"client_size\":[%ld,%ld]}\n",
         (long)pt.x,(long)pt.y,(long)(rc.right-rc.left),(long)(rc.bottom-rc.top));
  return 1;
}

static int send_ev(void *c, int dx, int dy, int b) {
  INPUT in;
  UINT n;
  DWORD err = 0, fp = 0;
  HWND f;
  (void)c;
  memset(&in, 0, sizeof in);
  in.type = INPUT_MOUSE;
  in.mi.dx = dx;
  in.mi.dy = dy;
  if (dx || dy) in.mi.dwFlags |= MOUSEEVENTF_MOVE;
  if (b == 'D') in.mi.dwFlags |= (g_swap ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN);
  if (b == 'U') in.mi.dwFlags |= (g_swap ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_LEFTUP);
  n = SendInput(1, &in, sizeof in);
  if (n != 1) err = GetLastError();
  f = GetForegroundWindow();
  if (f) GetWindowThreadProcessId(f, &fp);
  printf("{\"ev\":{\"dx\":%d,\"dy\":%d,\"b\":\"%c\",\"ret\":%u,\"err\":%lu,\"ms\":%llu,\"fgpid\":%lu}}\n",
         dx, dy, b, (unsigned)n, (unsigned long)err,
         (unsigned long long)(GetTickCount64() - g_t0), (unsigned long)fp);
  return (int)n;
}

static unsigned long long now_ms(void *c) { (void)c; return GetTickCount64(); }
static void sleep_ms(void *c, unsigned ms) { (void)c; Sleep(ms); }

static const char *preflight(void) {
  static wchar_t img[32768];
  DWORD sz = 32768;
  EW e;
  g_proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, g_t->pid);
  if (!g_proc) return "open process";
  if (!alive()) return "process identity";
  if (!QueryFullProcessImageNameW(g_proc, 0, img, &sz) || _wcsicmp(img, g_t->path))
    return "image path";
  if (!hash_ok()) return "image hash";
  e.pid = g_t->pid; e.n = 0; e.h = NULL;
  if (!EnumWindows(ew, (LPARAM)&e)) return "window enumeration";
  if (e.n != 1) return "window count";
  g_hwnd = e.h;
  if (((short)GetAsyncKeyState(VK_LBUTTON) | (short)GetAsyncKeyState(VK_RBUTTON)) < 0)
    return "button held";
  g_swap = GetSystemMetrics(SM_SWAPBUTTON) != 0;
  g_swap_known = 1;
  if (!desktop_ok()) return "desktop";
  if (!fg_ok()) { SetForegroundWindow(g_hwnd); Sleep(150); }
  return guard(NULL) ? NULL : g_why;
}

/* Foreground and cursor values are read-only metadata taken at receipt time. */
static void receipt(int live, const wh_result *r) {
  DWORD fp = 0;
  HWND f = GetForegroundWindow();
  POINT pt;
  char cur[48] = "null";
  if (f) GetWindowThreadProcessId(f, &fp);
  if (GetCursorPos(&pt)) snprintf(cur, sizeof cur, "[%ld,%ld]", (long)pt.x, (long)pt.y);
  printf("{\"live\":%s,\"state\":\"%s\",\"reason\":\"%s\",\"guard_detail\":\"%s\","
         "\"foreground_pid\":%lu,\"expected_pid\":%lu,\"mapping_known\":%s,\"swap_buttons\":%s,"
         "\"cursor\":%s,\"rows_done\":%u,\"attempted\":%u,"
         "\"delivered\":%u,\"cleanup_tried\":%u,\"release_owed\":%d,"
         "\"capture_verified\":%s,\"capture_stage\":\"%s\",\"capture_hresult\":%ld,"
         "\"guest_accepted\":false}\n",
         live ? "true" : "false", wh_status_name(r->state), r->reason, g_why,
         (unsigned long)fp, (unsigned long)(g_t ? g_t->pid : 0),
         g_swap_known ? "true" : "false",
         g_swap_known ? (g_swap ? "true" : "false") : "null", cur, r->rows_done,
         r->attempted, r->delivered, r->cleanup_tried, r->release_owed,
         g_cap.verified ? "true" : "false", g_cap.stage, g_cap.hr);
}

static int refuse(const char *m) { fprintf(stderr, "refused: %s\n", m); return 2; }

int main(int argc, char **argv) {
  static char buf[WH_MAX_BYTES + 1];
  const char *tn = NULL, *pf = NULL, *why;
  int live = 0, i, ti, st;
  size_t n;
  FILE *f;
  wh_plan pl;
  wh_result r;
  wh_backend be;
  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--live") && !live) live = 1;
    else if (!strcmp(argv[i], "--target") && !tn && i + 1 < argc) tn = argv[++i];
    else if (!strcmp(argv[i], "--plan") && !pf && i + 1 < argc) pf = argv[++i];
    else return refuse("arguments");
  }
  if (!tn || !pf) return refuse("missing flag");
  ti = !strcmp(tn, "snow") ? 0 : !strcmp(tn, "vbox") ? 1 : -1;
  if (ti < 0) return refuse("unknown target");
  if (wcscmp(wih_targets[0].role, L"snow") || wcscmp(wih_targets[1].role, L"vbox"))
    return refuse("target header roles");
  g_t = &wih_targets[ti];
  f = fopen(pf, "rb");
  if (!f) return refuse("plan unreadable");
  n = fread(buf, 1, sizeof buf, f);
  if (ferror(f)) { fclose(f); return refuse("plan read error"); }
  fclose(f);
  st = wh_parse(buf, n, &pl);
  if (st) { fprintf(stderr, "refused: plan %s\n", wh_status_name(st)); return 2; }
  be.ctx = NULL; be.now_ms = now_ms; be.sleep_ms = sleep_ms; be.guard = guard; be.send = send_ev;
  if (!live) {
    wh_run(&pl, &be, 1, &r);
    printf("{\"live\":false,\"valid\":true,\"rows\":%u,\"total_ms\":%u,\"plan\":[", pl.count, pl.total_ms);
    for (i = 0; i < (int)pl.count; i++)
      printf("%s[%d,%d,%u,\"%c\"]", i ? "," : "", pl.rows[i].dx, pl.rows[i].dy,
             pl.rows[i].delay, pl.rows[i].button);
    printf("]}\n");
    return 0;
  }
  memset(&r, 0, sizeof r);
  if (!g_t->pid) why = "inactive target";
  else why = preflight();
  if (why) {
    r.state = WH_E_TARGET; r.reason = why;
    receipt(1, &r);
    st = 3;
  } else {
    g_t0 = GetTickCount64();
    st = wh_run(&pl, &be, 0, &r);
    receipt(1, &r);
  }
  if (g_file) CloseHandle(g_file);
  if (g_proc) CloseHandle(g_proc);
  return st ? 3 : 0;
}
