/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 Dean Howell */
#include "helper.h"
#include <string.h>

const char *wh_status_name(int s) {
  static const char *const n[] = {"ok", "parse_error", "range_error", "order_error",
    "size_error", "guard_refused", "api_failure", "deadline", "target_refused"};
  return (s >= 0 && s <= WH_E_TARGET) ? n[s] : "unknown";
}

/* Checked integer: optional '-', at least one digit, no '+', bounded accumulation. */
static int num(const char **p, const char *e, int lo, int hi, int *out) {
  const char *s = *p;
  int neg = 0;
  unsigned digits = 0;
  long v = 0;
  if (s < e && *s == '-') { neg = 1; s++; }
  while (s < e && *s >= '0' && *s <= '9') {
    v = v * 10 + (*s - '0');
    if (v > 100000) return WH_E_RANGE;
    s++; digits++;
  }
  if (!digits) return WH_E_PARSE;
  if (neg) v = -v;
  if (v < lo || v > hi) return WH_E_RANGE;
  *out = (int)v;
  *p = s;
  return WH_OK;
}

static int lit(const char **p, const char *e, char c) {
  if (*p >= e || **p != c) return WH_E_PARSE;
  (*p)++;
  return WH_OK;
}

static int parse_row(const char **p, const char *e, wh_row *r) {
  int st, d;
  if ((st = num(p, e, -8, 8, &r->dx)) || (st = lit(p, e, ' '))) return st;
  if ((st = num(p, e, -8, 8, &r->dy)) || (st = lit(p, e, ' '))) return st;
  if ((st = num(p, e, 10, 1000, &d)) || (st = lit(p, e, ' '))) return st;
  r->delay = (unsigned)d;
  if (*p >= e || (**p != 'N' && **p != 'D' && **p != 'U')) return WH_E_PARSE;
  r->button = *(*p)++;
  if (*p < e && **p == '\r') (*p)++;
  return lit(p, e, '\n');
}

static int parse_all(const char *b, size_t n, wh_plan *pl) {
  const char *p, *e;
  int down = 0, used = 0, st;
  size_t i;
  if (!b || n == 0 || n > WH_MAX_BYTES) return WH_E_SIZE;
  p = b;
  e = b + n;
  for (i = 0; i < n; i++)
    if (b[i] == 0 || (unsigned char)b[i] > 126) return WH_E_PARSE;
  while (p < e) {
    wh_row *r;
    if (pl->count == WH_MAX_ROWS) return WH_E_SIZE;
    r = &pl->rows[pl->count];
    if ((st = parse_row(&p, e, r))) return st;
    pl->total_ms += r->delay;
    if (pl->total_ms > WH_MAX_TOTAL_MS) return WH_E_RANGE;
    if (r->button == 'D') {
      if (down || used) return WH_E_ORDER;
      down = 1;
    } else if (r->button == 'U') {
      if (!down) return WH_E_ORDER;
      down = 0; used = 1;
    }
    pl->count++;
  }
  return down ? WH_E_ORDER : WH_OK;
}

int wh_parse(const char *b, size_t n, wh_plan *pl) {
  int st;
  if (!pl) return WH_E_PARSE;
  memset(pl, 0, sizeof *pl);
  st = parse_all(b, n, pl);
  if (st) memset(pl, 0, sizeof *pl);
  return st;
}

/* Cleanup is guarded like any event; a failing guard means an uncertain target, so no event. */
static void cleanup(const wh_backend *be, wh_result *r) {
  if (!r->release_owed) return;
  r->cleanup_tried = 1;
  if (!be->guard(be->ctx)) return;
  r->attempted++;
  if (be->send(be->ctx, 0, 0, 'U') == 1) { r->delivered++; r->release_owed = 0; }
}

static int fail(const wh_backend *be, wh_result *r, int st, int clean) {
  r->state = st;
  r->reason = wh_status_name(st);
  if (clean) cleanup(be, r);
  return st;
}

/* Pure bounded revalidation of every row; the public plan fields are mutable. */
static int valid_plan(const wh_plan *pl) {
  unsigned i, sum = 0;
  int down = 0, used = 0;
  if (pl->count == 0 || pl->count > WH_MAX_ROWS) return WH_E_SIZE;
  for (i = 0; i < pl->count; i++) {
    const wh_row *w = &pl->rows[i];
    if (w->dx < -8 || w->dx > 8 || w->dy < -8 || w->dy > 8 ||
        w->delay < 10 || w->delay > 1000) return WH_E_RANGE;
    sum += w->delay;
    if (w->button == 'D') {
      if (down || used) return WH_E_ORDER;
      down = 1;
    } else if (w->button == 'U') {
      if (!down) return WH_E_ORDER;
      down = 0; used = 1;
    } else if (w->button != 'N') return WH_E_PARSE;
  }
  if (sum > WH_MAX_TOTAL_MS || sum != pl->total_ms) return WH_E_RANGE;
  return down ? WH_E_ORDER : WH_OK;
}

int wh_run(const wh_plan *pl, const wh_backend *be, int dry, wh_result *r) {
  unsigned long long start;
  unsigned i;
  int st;
  if (!r) return WH_E_API;
  memset(r, 0, sizeof *r);
  r->reason = wh_status_name(WH_OK);
  if (!pl) return fail(NULL, r, WH_E_SIZE, 0);
  if ((st = valid_plan(pl))) return fail(NULL, r, st, 0);
  if (dry) return WH_OK;
  if (!be || !be->now_ms || !be->sleep_ms || !be->guard || !be->send)
    return fail(NULL, r, WH_E_API, 0);
  start = be->now_ms(be->ctx);
  for (i = 0; i < pl->count; i++) {
    const wh_row *w = &pl->rows[i];
    int n;
    be->sleep_ms(be->ctx, w->delay);
    if (be->now_ms(be->ctx) - start > WH_MAX_TOTAL_MS) return fail(be, r, WH_E_DEADLINE, 1);
    if (!be->guard(be->ctx)) return fail(be, r, WH_E_GUARD, 0);
    /* The guard may consume host time; re-check before any planned event or no-event completion. */
    if (be->now_ms(be->ctx) - start > WH_MAX_TOTAL_MS) return fail(be, r, WH_E_DEADLINE, 1);
    if (w->button == 'N' && !w->dx && !w->dy) { r->rows_done++; continue; }
    if (w->button == 'D') r->release_owed = 1;
    r->attempted++;
    n = be->send(be->ctx, w->dx, w->dy, w->button);
    if (n != 1) return fail(be, r, WH_E_API, 1);
    r->delivered++;
    if (w->button == 'U') r->release_owed = 0;
    r->rows_done++;
  }
  return WH_OK;
}
