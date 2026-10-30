/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 Dean Howell. Test-only simulated interfaces. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int wih_context_snapshot(HWND h,DWORD p,FILE *out){const char *s=getenv("WIH_CASE");(void)h;(void)p;if(s && strcmp(s,"snapshot_refuse")==0)return 1;if(s && strcmp(s,"snapshot_error")==0)return -1;return fprintf(out,"{\"synthetic\":true}\n")<0?-1:0;}
