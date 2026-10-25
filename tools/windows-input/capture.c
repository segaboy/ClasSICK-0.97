/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 Dean Howell
 * Read-only: queries only the bound window's UIA subtree for the captured-indicator name. */
#define COBJMACROS
#include <initguid.h>
#include <windows.h>
#include <objbase.h>
#include <oleauto.h>
#include <uiautomation.h>
#include <string.h>
#include "capture.h"

#define CAP_NAME L"Mouse status-bar indicator, Pointer is captured"

static int fail(wh_capture *o, const char *stage, HRESULT hr) {
  o->verified = 0;
  o->stage = stage;
  o->hr = (long)hr;
  return 0;
}

static void query(HWND hwnd, DWORD pid, wh_capture *o) {
  IUIAutomation2 *ua = NULL;
  IUIAutomationElement *root = NULL, *found = NULL;
  IUIAutomationCondition *cond = NULL;
  VARIANT v;
  BSTR name = NULL;
  int p = 0;
  BOOL off = TRUE;
  HRESULT hr;

  VariantInit(&v);
  hr = CoCreateInstance(&CLSID_CUIAutomation8, NULL, CLSCTX_INPROC_SERVER,
                        &IID_IUIAutomation2, (void **)&ua);
  if (FAILED(hr) || !ua) { fail(o, "create", FAILED(hr) ? hr : E_FAIL); goto done; }
  hr = IUIAutomation2_put_ConnectionTimeout(ua, 1000);
  if (FAILED(hr)) { fail(o, "connection timeout", hr); goto done; }
  hr = IUIAutomation2_put_TransactionTimeout(ua, 1000);
  if (FAILED(hr)) { fail(o, "transaction timeout", hr); goto done; }
  hr = IUIAutomation2_ElementFromHandle(ua, (UIA_HWND)hwnd, &root);
  if (FAILED(hr) || !root) { fail(o, "root", FAILED(hr) ? hr : E_FAIL); goto done; }
  hr = IUIAutomationElement_get_CurrentProcessId(root, &p);
  if (FAILED(hr)) { fail(o, "root pid", hr); goto done; }
  if ((DWORD)p != pid) { fail(o, "root pid mismatch", S_FALSE); goto done; }
  v.vt = VT_BSTR;
  v.bstrVal = SysAllocString(CAP_NAME);
  if (!v.bstrVal) { v.vt = VT_EMPTY; fail(o, "alloc", E_OUTOFMEMORY); goto done; }
  hr = IUIAutomation2_CreatePropertyCondition(ua, UIA_NamePropertyId, v, &cond);
  if (FAILED(hr) || !cond) { fail(o, "condition", FAILED(hr) ? hr : E_FAIL); goto done; }
  hr = IUIAutomationElement_FindFirst(root, TreeScope_Descendants, cond, &found);
  if (FAILED(hr)) { fail(o, "find", hr); goto done; }
  if (!found) { fail(o, "indicator absent", S_FALSE); goto done; }
  hr = IUIAutomationElement_get_CurrentIsOffscreen(found, &off);
  if (FAILED(hr)) { fail(o, "offscreen", hr); goto done; }
  if (off) { fail(o, "indicator offscreen", S_FALSE); goto done; }
  p = 0;
  hr = IUIAutomationElement_get_CurrentProcessId(found, &p);
  if (FAILED(hr)) { fail(o, "found pid", hr); goto done; }
  if ((DWORD)p != pid) { fail(o, "found pid mismatch", S_FALSE); goto done; }
  hr = IUIAutomationElement_get_CurrentName(found, &name);
  if (FAILED(hr)) { fail(o, "name", hr); goto done; }
  if (!name || wcscmp(name, CAP_NAME)) { fail(o, "name mismatch", S_FALSE); goto done; }
  o->verified = 1;
  o->stage = "verified";
  o->hr = (long)S_OK;
done:
  if (name) SysFreeString(name);
  VariantClear(&v);
  if (found) IUIAutomationElement_Release(found);
  if (cond) IUIAutomationCondition_Release(cond);
  if (root) IUIAutomationElement_Release(root);
  if (ua) IUIAutomation2_Release(ua);
}

void wh_capture_query(void *hwnd, unsigned long pid, wh_capture *out) {
  HRESULT hr;
  if (!out) return;
  out->verified = 0;
  out->stage = "argument";
  out->hr = (long)E_INVALIDARG;
  if (!hwnd || !pid) return;
  out->stage = "init";
  out->hr = 0;
  hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (hr != S_OK && hr != S_FALSE) { fail(out, "coinit", hr); return; }
  query((HWND)hwnd, (DWORD)pid, out);
  CoUninitialize();
}
