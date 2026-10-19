# SPEC-0050: Continuous native PC preview session

SPDX-License-Identifier: GPL-3.0-or-later. Copyright (C) 2026 Dean Howell.

Status: engineering profile `paired-native-preview-1` (2026-10-19). Related:
ADR-0056, IMPL-0053, TEST-0059. SPEC-0049 stays as written, with its original
date, and links forward to this document.

## Scope
The explicit PC outline preview gets the same no-normal-timeout behavior
as the Mac native 68000 path. The Mac path does not change. This is not a guarantee
of unlimited running time, desktop behavior or hardware behavior. It makes no claim about the cause of
earlier failed human checks. Synthetic passes do not show that the preview is reliable for human users.

## Profile
- `CLASSICK_PC_PREVIEW_SESSION` is a compile flag that applies only to
  `platform/uefi/keyboard.c`. It defaults to 0.
- A value other than 0 or 1 is a compile error. A value of 1 is a compile error unless
  `CLASSICK_PC_MOUSE`, `CLASSICK_OUTLINE_DRAWING` and
  `CLASSICK_SELECTED_DRAWING` are all 1.
- The native build sets the flag to 1 only when `-OutlineDrawing` is selected.
  The existing outline and diagnostic builds without the flag are unchanged.
- No ABI, header, signature, UART, PS/2, queue, scene, drawing-core or Mac
  changes. UART callers still pass the nominal value 60.

## Validation
Argument checks are unchanged: `seconds` must be 1..3600. All existing startup gates stay:
NOT_EXITED, NOT_READY, TIMER, NO_8042, TARGET, ARENA and VALUE.

## Behavior with the flag set to 1
- No normal return when `seconds` elapses. The progress step stays at 0.
- Space counts once per non-repeat press. The Space offset may still appear in the
  trace step metadata, but outline render progress stays 0.
- Mouse left-button presses are counted on the edge. Held reports do not add to the count.
- Escape increments the escape counter but does not end the session.
- The scene is redrawn only when the shown input count or the step changes.

## Termination
The host ends the session by closing it or powering off. The following failures
still return with the existing fail-closed results and trace records: hardware,
controller, timer value, time overflow, stall, queue and draw.

## Limits
The bounds come from elapsed-time overflow in `cs_pmtimer_time` and from saturating
counters. Old build modes keep the 60-second deadline, the Escape exit and their
records.

## Test cases (TEST-0059, tests/native-preview-session.c)
1. Argument and startup failures: a NULL trace, seconds 0 and 3601, not exited,
   not ready, no 8042, and a failed controller self-test.
2. Real loop with seconds=60. Mouse press, held and release, Space, Escape and Space are
   sent after an elapsed time of at least 61 s. The test checks: still running; one static frame before
   the input; counts 3/1; trace padding, guard bytes and pitch padding intact; and every byte outside the exact
   centered counter rectangle unchanged and changed counter bytes.
3. The same run with seconds=1, which shows there is no normal cutoff.
4. Both runs end with an injected 24-bit timer value fault after at least 65 s, which must
   give `CS_KBD_VALUE`. Without the flag, the old behavior fails the test instead
   of hanging.
5. The existing native-pointer and native-keyboard tests cover the legacy timed and Escape
   behavior.
