/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 Dean Howell
 * Read-only UIAutomation accessibility-metadata capture verifier. Windows 11 host only. */
#ifndef WIH_CAPTURE_H
#define WIH_CAPTURE_H

typedef struct {
  int verified;       /* 1 only if every check passed */
  const char *stage;  /* last stage reached or failing stage */
  long hr;            /* HRESULT of that stage */
} wh_capture;

/* hwnd is the bound window handle (as void *), pid the bound process id. */
void wh_capture_query(void *hwnd, unsigned long pid, wh_capture *out);
#endif
