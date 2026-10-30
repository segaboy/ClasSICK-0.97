/* Original read-only context snapshot caller; GPL-3.0-or-later. Never calls helper main, preflight or input.
 *
 * STDOUT CONTRACT: anything written to stdout is valid ONLY if this program
 * exits with status 0. The snapshot is written before the final ownership
 * verification, so a nonzero exit means stdout must be discarded.
 */
#define main unused_native_cli
#include "native.c"
#undef main

/* Returns 0 on success, 1 on refusal, -1 on error. */
int wih_context_snapshot(HWND, DWORD, FILE *);

static int fail(const char *why) {
  fprintf(stderr, "{\"read_only\":true,\"ok\":false,\"refusal\":\"%s\"}\n", why);
  return 1;
}

static int path_ok(void) {
  static WCHAR buf[32768];
  DWORD n = (DWORD)(sizeof buf / sizeof buf[0]);
  if (!QueryFullProcessImageNameW(g_proc, 0, buf, &n)) return 0;
  return _wcsicmp(buf, g_t->path) == 0;
}

/* Point-in-time checks only; ownership is not claimed atomic across the next instant. */
static const char *verify(HWND expect, HWND *out) {
  EW e;
  DWORD pid = 0;
  if (!alive()) return "process identity";
  if (!path_ok()) return "image path";
  if (!desktop_ok()) return "input desktop";
  e.pid = g_t->pid; e.n = 0; e.h = NULL;
  if (!EnumWindows(ew, (LPARAM)&e)) return "enumeration";
  if (e.n != 1 || !e.h) return "owned window count";
  if (expect && e.h != expect) return "window changed";
  GetWindowThreadProcessId(e.h, &pid);
  if (pid != g_t->pid) return "window pid";
  *out = e.h;
  return NULL;
}

int main(void) {
  HWND h = NULL, h2 = NULL;
  const char *why;
  int rc = 1, snap;
  g_t = &wih_targets[1];
  if (wcscmp(g_t->role, L"vbox") != 0) { rc = fail("role"); goto done; }
  g_proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, g_t->pid);
  if (!g_proc) { rc = fail("open process"); goto done; }
  if (!hash_ok()) { rc = fail("image hash"); goto done; }
  if ((why = verify(NULL, &h)) != NULL) { rc = fail(why); goto done; }
  snap = wih_context_snapshot(h, g_t->pid, stdout);
  if (fflush(stdout) != 0) { rc = fail("stdout flush"); goto done; }
  /* Final ownership verification; stdout is trustworthy only if we reach rc = 0. */
  if ((why = verify(h, &h2)) != NULL) { rc = fail(why); goto done; }
  if (snap != 0) { rc = fail(snap == 1 ? "snapshot refused" : "snapshot error"); goto done; }

  /* Close the first hash handle before re-verifying so it is not overwritten/leaked. */
  if (g_file && g_file != INVALID_HANDLE_VALUE) {
    HANDLE f = g_file;
    g_file = NULL;
    if (!CloseHandle(f)) { rc = fail("close image file"); goto done; }
  } else {
    g_file = NULL;
  }
  if (!hash_ok()) { rc = fail("final image hash"); goto done; }
  /* Point-in-time recheck after the final hash; not atomic. */
  if ((why = verify(h, &h2)) != NULL) { rc = fail(why); goto done; }
  rc = 0;
done:
  if (g_file && g_file != INVALID_HANDLE_VALUE) {
    HANDLE f = g_file;
    g_file = NULL;
    if (!CloseHandle(f)) rc = fail("close image file");
  }
  if (g_proc && g_proc != INVALID_HANDLE_VALUE) {
    HANDLE p = g_proc;
    g_proc = NULL;
    if (!CloseHandle(p)) rc = fail("close process");
  }
  return rc;
}
