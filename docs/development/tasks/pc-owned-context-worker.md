Windows 11, ClasSICK 0.97. Fresh clean-room worker: use only this prompt, original source below, and the interface facts given. Do not browse, run commands, edit files or invoke tools beyond reading this prompt. Exclude all Microsoft/Apple implementation examples, manual samples, emulator source, ROM/media, private material and credentials. Return GPL-3.0-or-later original code only.

Task: correct the original read-only owned-window caller below. It includes project-owned native.c to use project functions. Its data inputs are a caller-frozen 	est-targets.h with exact vbox PID, creation time, executable path and SHA-256. hash_ok() opens g_t->path into g_file, hashes that open file, and returns nonzero only on hash match. It leaves g_file open even on success. live() verifies the process is alive and creation time matches. path_ok() compares the queried process path to the pinned path. desktop_ok() confirms the default input desktop. w() enumerates visible positive-client windows for exactly the pinned PID, including the project's target-window filter. Snapshot returns 0 for success, 1 refusal, -1 error and does not inject input or activate any window. All stdout is invalid unless program exits zero.

Required fix: independently verify the pinned executable hash again AFTER snapshot and final identity/window checks; before calling hash_ok() a second time, close the first g_file and set it NULL so the open handle is not leaked/overwritten. Refuse if either close or final hash verification fails; use one cleanup path and correctly close all valid handles on all exits, treating handle-close failure as failure. Recheck alive/path/desktop and same owned window once more after final hash, before success; do not claim atomicity. No live action, network, screenshot, process-memory access, guest/device input, window activation, desktop setting change or extra process enumeration. Never invoke preflight(), helper main, SendInput or window manipulation. Preserve role using wcscmp(..., L\"vbox\") and success snap == 0. Any stdout is discarded if status nonzero. Use the same includes and symbols. Return JSON with source as the complete source string, and handoff containing result, tests_executed=[], blockers and decisions_needed, maximum 150 lines. You may propose tests but do not claim execution.

Original project-owned caller:
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
  fflush(stdout);
  /* Final ownership verification; stdout is trustworthy only if we reach rc = 0. */
  if ((why = verify(h, &h2)) != NULL) { rc = fail(why); goto done; }
  if (snap != 0) { rc = fail(snap == 1 ? "snapshot refused" : "snapshot error"); goto done; }
  rc = 0;
done:
  if (g_file && g_file != INVALID_HANDLE_VALUE) { CloseHandle(g_file); g_file = NULL; }
  if (g_proc && g_proc != INVALID_HANDLE_VALUE) { CloseHandle(g_proc); g_proc = NULL; }
  return rc;
}
