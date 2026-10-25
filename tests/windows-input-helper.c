/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 Dean Howell */
#include <stdio.h>
#include <string.h>
#include "helper.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

typedef struct {
  unsigned long long t;
  unsigned slow, gslow;
  int guards, failfrom, sends, cb, rets[8], log[8][3];
} F;

static unsigned long long fnow(void *c) { F *f = c; f->cb++; return f->t; }
static void fsleep(void *c, unsigned ms) { F *f = c; f->cb++; f->t += ms + f->slow; }
static int fguard(void *c) {
  F *f = c;
  f->cb++; f->guards++;
  f->t += f->gslow;
  return !(f->failfrom && f->guards >= f->failfrom);
}
static int fsend(void *c, int dx, int dy, int b) {
  F *f = c;
  int i = f->sends < 8 ? f->sends : 7;
  f->cb++;
  f->log[i][0] = dx; f->log[i][1] = dy; f->log[i][2] = b;
  f->sends++;
  return f->rets[i];
}

static void setup(F *f, wh_backend *be) {
  int i;
  memset(f, 0, sizeof *f);
  for (i = 0; i < 8; i++) f->rets[i] = 1;
  be->ctx = f; be->now_ms = fnow; be->sleep_ms = fsleep; be->guard = fguard; be->send = fsend;
}

static int ps(const char *s) { wh_plan p; return wh_parse(s, strlen(s), &p); }

/* Run a live plan with preset failure knobs. */
static int runk(const char *s, wh_result *r, F *f, int failfrom, int ret0, int ret1, int ret2, unsigned slow) {
  wh_plan p;
  wh_backend be;
  setup(f, &be);
  f->failfrom = failfrom; f->rets[0] = ret0; f->rets[1] = ret1; f->rets[2] = ret2; f->slow = slow;
  if (wh_parse(s, strlen(s), &p)) return -1;
  return wh_run(&p, &be, 0, r);
}

/* Mutate a valid parsed plan; dry and live must both refuse with zero callbacks. */
static int mrun(int what, long long v) {
  wh_plan p;
  wh_backend be;
  wh_result r;
  F f;
  int st, st2;
  const char *ok = "0 0 10 N\n1 0 10 D\n0 2 10 U\n";
  setup(&f, &be);
  if (wh_parse(ok, strlen(ok), &p)) return -99;
  switch (what) {
    case 0: p.rows[1].dx = (int)v; break;
    case 1: p.rows[2].dy = (int)v; break;
    case 2: p.rows[0].delay = (unsigned)v; break;
    case 3: p.rows[0].button = (char)v; break;
    case 4: p.rows[2].button = (char)v; break;
    case 5: p.total_ms = (unsigned)v; break;
    default: p.count = (unsigned)v; break;
  }
  st = wh_run(&p, &be, 1, &r);
  st2 = wh_run(&p, &be, 0, &r);
  CHECK(f.cb == 0 && f.sends == 0 && st == st2);
  return st;
}

static void rows(char *b, int n, const char *row) {
  int i;
  b[0] = 0;
  for (i = 0; i < n; i++) strcat(b, row);
}

int main(void) {
  static char big[4098];
  wh_plan p;
  wh_result r;
  wh_backend be;
  F f;
  const char *ok = "0 0 10 N\n1 0 10 D\n0 2 10 U\n";
  const char nul[] = "0 0 10\0N\n";

  /* parser: accepted */
  CHECK(wh_parse(ok, strlen(ok), &p) == WH_OK);
  CHECK(p.count == 3 && p.total_ms == 30 && p.rows[1].dx == 1 && p.rows[1].button == 'D');
  CHECK(p.rows[2].dy == 2 && p.rows[2].button == 'U');
  CHECK(ps("-8 8 10 N\r\n") == WH_OK);
  CHECK(ps("0 0 10 N\n0 0 1000 N\n") == WH_OK);
  rows(big, 64, "0 0 10 N\n"); CHECK(ps(big) == WH_OK);
  rows(big, 30, "0 0 1000 N\n"); CHECK(ps(big) == WH_OK);
  /* parser: refused */
  CHECK(wh_parse("", 0, &p) == WH_E_SIZE && p.count == 0);
  rows(big, 65, "0 0 10 N\n"); CHECK(ps(big) == WH_E_SIZE);
  rows(big, 31, "0 0 1000 N\n"); CHECK(ps(big) == WH_E_RANGE);
  memset(big, 'N', 4097); CHECK(wh_parse(big, 4097, &p) == WH_E_SIZE);
  CHECK(ps("0 0 99999999999999999999 N\n") == WH_E_RANGE);
  CHECK(ps("9 0 10 N\n") == WH_E_RANGE);
  CHECK(ps("0 -9 10 N\n") == WH_E_RANGE);
  CHECK(ps("0 0 9 N\n") == WH_E_RANGE);
  CHECK(ps("0 0 1001 N\n") == WH_E_RANGE);
  CHECK(ps("0 0 10 N") == WH_E_PARSE);
  CHECK(ps("0 0 10 N\nx") == WH_E_PARSE);
  CHECK(ps("0 0 10 N\n\n") == WH_E_PARSE);
  CHECK(ps("0 0 10 N \n") == WH_E_PARSE);
  CHECK(ps("0  0 10 N\n") == WH_E_PARSE);
  CHECK(ps("0\t0 10 N\n") == WH_E_PARSE);
  CHECK(ps("+1 0 10 N\n") == WH_E_PARSE);
  CHECK(ps("- 0 10 N\n") == WH_E_PARSE);
  CHECK(ps("0 0 10 X\n") == WH_E_PARSE);
  CHECK(ps("0 0 10 n\n") == WH_E_PARSE);
  CHECK(ps("0 0 10 \xC3\n") == WH_E_PARSE);
  CHECK(wh_parse(nul, sizeof nul - 1, &p) == WH_E_PARSE);
  CHECK(ps("0 0 10 U\n") == WH_E_ORDER);
  CHECK(ps("0 0 10 D\n") == WH_E_ORDER);
  CHECK(ps("0 0 10 D\n0 0 10 D\n0 0 10 U\n") == WH_E_ORDER);
  CHECK(ps("0 0 10 D\n0 0 10 U\n0 0 10 D\n0 0 10 U\n") == WH_E_ORDER);
  CHECK(ps("0 0 10 D\n0 0 10 U\n0 0 10 U\n") == WH_E_ORDER);

  /* malformed and dry-run input never reach the backend */
  setup(&f, &be);
  CHECK(wh_parse("0 0 10 D\n", 9, &p) == WH_E_ORDER && p.count == 0);
  CHECK(wh_run(&p, &be, 0, &r) == WH_E_SIZE && f.cb == 0);
  CHECK(wh_parse(ok, strlen(ok), &p) == WH_OK);
  CHECK(wh_run(&p, &be, 1, &r) == WH_OK && f.cb == 0 && r.attempted == 0);

  /* clean run: one send per event, N/0/0 is a control, final U leaves nothing owed */
  CHECK(runk(ok, &r, &f, 0, 1, 1, 1, 0) == WH_OK);
  CHECK(f.sends == 2 && f.guards == 3 && f.t == 30);
  CHECK(f.log[0][0] == 1 && f.log[0][1] == 0 && f.log[0][2] == 'D');
  CHECK(f.log[1][0] == 0 && f.log[1][1] == 2 && f.log[1][2] == 'U');
  CHECK(r.attempted == 2 && r.delivered == 2 && r.release_owed == 0 && r.rows_done == 3);
  CHECK(runk("0 0 10 N\n0 0 20 N\n", &r, &f, 0, 1, 1, 1, 0) == WH_OK);
  CHECK(f.sends == 0 && f.guards == 2 && f.t == 30 && r.attempted == 0);
  CHECK(runk("3 -4 10 N\n", &r, &f, 0, 1, 1, 1, 0) == WH_OK);
  CHECK(f.sends == 1 && f.log[0][0] == 3 && f.log[0][1] == -4 && f.log[0][2] == 'N');

  /* foreground/guard loss */
  CHECK(runk("1 0 10 D\n0 0 10 U\n", &r, &f, 1, 1, 1, 1, 0) == WH_E_GUARD);
  CHECK(f.sends == 0 && r.release_owed == 0 && r.attempted == 0);
  CHECK(runk("1 0 10 D\n0 0 10 U\n", &r, &f, 2, 1, 1, 1, 0) == WH_E_GUARD);
  CHECK(f.sends == 1 && r.release_owed == 1 && r.cleanup_tried == 0 && f.guards == 2);

  /* partial or zero native result, with cleanup success and failure */
  CHECK(runk("0 0 10 D\n1 0 10 U\n", &r, &f, 0, 0, 1, 1, 0) == WH_E_API);
  CHECK(f.sends == 2 && f.log[1][2] == 'U' && r.attempted == 2 && r.delivered == 1);
  CHECK(r.release_owed == 0 && r.cleanup_tried == 1);
  CHECK(runk("0 0 10 D\n1 0 10 U\n", &r, &f, 0, 1, 0, 0, 0) == WH_E_API);
  CHECK(f.sends == 3 && r.attempted == 3 && r.delivered == 1 && r.release_owed == 1);
  CHECK(runk("0 0 10 D\n1 0 10 U\n", &r, &f, 0, 1, 2, 1, 0) == WH_E_API);
  CHECK(r.delivered == 2 && f.sends == 3);
  CHECK(runk("1 0 10 N\n", &r, &f, 0, 0, 1, 1, 0) == WH_E_API);
  CHECK(f.sends == 1 && r.release_owed == 0 && r.cleanup_tried == 0);

  /* deadline: guarded cleanup succeeds, or is refused and stays owed */
  CHECK(runk("0 0 10 D\n0 0 10 N\n0 0 10 U\n", &r, &f, 0, 1, 1, 1, 20000) == WH_E_DEADLINE);
  CHECK(f.sends == 2 && f.log[1][2] == 'U' && r.release_owed == 0 && r.cleanup_tried == 1);
  CHECK(runk("0 0 10 D\n0 0 10 N\n0 0 10 U\n", &r, &f, 2, 1, 1, 1, 20000) == WH_E_DEADLINE);
  CHECK(f.sends == 1 && r.release_owed == 1 && r.cleanup_tried == 1 && f.guards == 2);
  CHECK(runk("0 0 10 N\n", &r, &f, 0, 1, 1, 1, 31000) == WH_E_DEADLINE);
  CHECK(f.sends == 0 && f.guards == 0);

  CHECK(strcmp(wh_status_name(WH_E_ORDER), "order_error") == 0);

  /* NULL arguments never dereference */
  CHECK(wh_parse(NULL, 1, &p) == WH_E_SIZE);
  CHECK(wh_parse(ok, strlen(ok), NULL) == WH_E_PARSE);
  CHECK(wh_run(NULL, &be, 0, &r) == WH_E_SIZE);
  CHECK(wh_parse(ok, strlen(ok), &p) == WH_OK);
  setup(&f, &be);
  CHECK(wh_run(&p, &be, 0, NULL) == WH_E_API);
  CHECK(wh_run(&p, NULL, 0, &r) == WH_E_API);
  be.send = NULL; CHECK(wh_run(&p, &be, 0, &r) == WH_E_API && f.cb == 0);
  setup(&f, &be); be.guard = NULL; CHECK(wh_run(&p, &be, 0, &r) == WH_E_API && f.cb == 0);
  setup(&f, &be); be.now_ms = NULL; CHECK(wh_run(&p, &be, 0, &r) == WH_E_API && f.cb == 0);
  setup(&f, &be); be.sleep_ms = NULL; CHECK(wh_run(&p, &be, 0, &r) == WH_E_API && f.cb == 0);
  CHECK(wh_run(&p, NULL, 1, &r) == WH_OK && r.attempted == 0);

  /* mutated public plans are refused before any callback */
  CHECK(mrun(0, 9) == WH_E_RANGE);
  CHECK(mrun(0, -9) == WH_E_RANGE);
  CHECK(mrun(1, 9) == WH_E_RANGE);
  CHECK(mrun(2, 9) == WH_E_RANGE);
  CHECK(mrun(2, 1001) == WH_E_RANGE);
  CHECK(mrun(2, 4294967295LL) == WH_E_RANGE);
  CHECK(mrun(2, 11) == WH_E_RANGE);
  CHECK(mrun(3, 'X') == WH_E_PARSE);
  CHECK(mrun(3, 'D') == WH_E_ORDER);
  CHECK(mrun(3, 'U') == WH_E_ORDER);
  CHECK(mrun(4, 'N') == WH_E_ORDER);
  CHECK(mrun(5, 31) == WH_E_RANGE);
  CHECK(mrun(5, 0) == WH_E_RANGE);
  CHECK(mrun(5, 4294967295LL) == WH_E_RANGE);
  CHECK(mrun(6, 0) == WH_E_SIZE);
  CHECK(mrun(6, 65) == WH_E_SIZE);

  /* a guard that consumes the deadline stops planned events and no-event rows */
  setup(&f, &be);
  CHECK(wh_parse("0 0 10 N\n", 9, &p) == WH_OK);
  f.gslow = 31000;
  CHECK(wh_run(&p, &be, 0, &r) == WH_E_DEADLINE && f.sends == 0 && f.guards == 1);
  setup(&f, &be);
  CHECK(wh_parse("1 0 10 N\n", 9, &p) == WH_OK);
  f.gslow = 31000;
  CHECK(wh_run(&p, &be, 0, &r) == WH_E_DEADLINE && f.sends == 0 && r.attempted == 0);
  setup(&f, &be);
  CHECK(wh_parse("0 0 10 D\n0 0 10 N\n0 0 10 U\n", 27, &p) == WH_OK);
  f.gslow = 20000;
  CHECK(wh_run(&p, &be, 0, &r) == WH_E_DEADLINE);
  CHECK(f.sends == 2 && f.log[1][2] == 'U' && f.guards == 3 && r.release_owed == 0);

  printf("failures: %d\n", fails);
  return fails != 0;
}
