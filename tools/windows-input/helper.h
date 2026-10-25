/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 Dean Howell */
#ifndef WIH_HELPER_H
#define WIH_HELPER_H
#include <stddef.h>

#define WH_MAX_ROWS 64
#define WH_MAX_BYTES 4096
/* Requested delay sum is <=30000 ms. Host sleeps, clock reads and guards can still take longer in
 * wall time; the deadline is checked after each sleep and each guard, and planned events stop once
 * it is exceeded. */
#define WH_MAX_TOTAL_MS 30000

enum { WH_OK = 0, WH_E_PARSE, WH_E_RANGE, WH_E_ORDER, WH_E_SIZE,
       WH_E_GUARD, WH_E_API, WH_E_DEADLINE, WH_E_TARGET };

typedef struct { int dx, dy; unsigned delay; char button; } wh_row;
typedef struct { wh_row rows[WH_MAX_ROWS]; unsigned count, total_ms; } wh_plan;

/* send returns the native event count; anything but 1 is a failure. guard returns nonzero if safe. */
typedef struct {
  void *ctx;
  unsigned long long (*now_ms)(void *);
  void (*sleep_ms)(void *, unsigned);
  int (*guard)(void *);
  int (*send)(void *, int dx, int dy, int button);
} wh_backend;

typedef struct {
  int state;
  const char *reason;
  unsigned rows_done, attempted, delivered, cleanup_tried;
  int release_owed;
} wh_result;

int wh_parse(const char *buf, size_t len, wh_plan *plan);
int wh_run(const wh_plan *plan, const wh_backend *be, int dry, wh_result *res);
const char *wh_status_name(int status);
#endif
