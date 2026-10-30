/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 Dean Howell. Test-only simulated interfaces. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
static const char *scenario;
static int hashes, verifies, closes_file, closes_process;
typedef struct { const wchar_t *role, *path; unsigned char sha256[32]; DWORD pid; unsigned long long created; } wih_target;
static const wih_target wih_targets[2]={{L"snow",L"ignored",{0},0,0},{L"vbox",L"fixture",{0},7,42}};
static HANDLE g_proc,g_file; static const wih_target *g_t;
typedef struct { DWORD pid; int n; HWND h; } EW;
static int mode(const char *s){return scenario && strcmp(scenario,s)==0;}
static int alive(void){return !(mode("late_dead") && verifies>=2);}
static int desktop_ok(void){return !mode("desktop");}
static BOOL CALLBACK ew(HWND h,LPARAM p){(void)h;(void)p;return TRUE;}
static HANDLE fake_open(DWORD a,BOOL b,DWORD p){(void)a;(void)b;if(p!=7 || mode("open"))return NULL;return (HANDLE)(UINT_PTR)2;}
static BOOL fake_close(HANDLE h){if(h==(HANDLE)(UINT_PTR)1){++closes_file;if(mode("close_first")&&closes_file==1)return FALSE;if(mode("close_final")&&closes_file==2)return FALSE;return TRUE;}if(h==(HANDLE)(UINT_PTR)2){++closes_process;return !mode("close_process");}return FALSE;}
static int hash_ok(void){++hashes;g_file=(HANDLE)(UINT_PTR)1;return !(mode("hash_first")&&hashes==1)&&!(mode("hash_final")&&hashes==2);}
static BOOL fake_path(HANDLE h,DWORD f,LPWSTR out,PDWORD n){const wchar_t *s=mode("path")?L"wrong":L"fixture";size_t len=wcslen(s);(void)h;(void)f;if(*n<=len)return FALSE;wcscpy(out,s);*n=(DWORD)len;return TRUE;}
static BOOL fake_enum(WNDENUMPROC cb,LPARAM lp){EW *e=(EW*)lp;(void)cb;++verifies;e->n=mode("count")?2:1;e->h=(HWND)(UINT_PTR)((mode("window_changed")&&verifies==2)||(mode("final_window_changed")&&verifies==3)?3:1);return TRUE;}
static DWORD fake_pid(HWND h,LPDWORD p){(void)h;*p=mode("pid")?8:7;return 4;}
#define OpenProcess fake_open
#define CloseHandle fake_close
#define QueryFullProcessImageNameW fake_path
#define EnumWindows fake_enum
#define GetWindowThreadProcessId fake_pid
/* Test-only setup: not a real Windows target. */
static void report(void){fprintf(stderr,"COUNTS %d %d %d %d\n",hashes,verifies,closes_file,closes_process);}
static void init_case(void){scenario=getenv("WIH_CASE");atexit(report);}
