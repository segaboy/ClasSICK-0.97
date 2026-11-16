<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Paired preview qualification work

2026-11-16; continuation from `8b72181`. SRC-0608 / SPEC-0109 /
IMPL-0138 / TEST-0196. Codex implementation and self-verification under the
independent-implementation policy; no independent human review asserted.
No Apple/Microsoft implementation, firmware, game code or new third-party
implementation input was used. Existing unrelated edits remain preserved.

## Buffered Mac input

The new `--window-buffered-collect` build option captures into a 128-entry
FIFO in the interrupt and decodes in foreground. Packed aligned samples and
explicit five-word copies avoid odd accesses and the byte-copy runtime cost.
Foreground owns both normalized inbox sides. FIFO overflow cancels input and
requires a fresh baseline; it cannot masquerade as lossless motion.
The original full-register interrupt veneers remain selected in the retained
build. C-ABI-save experiments were measured but are not required by it.

Retained twin build:
`C:/Repos/.task-temp/mac-buffered-20261116-f/manifest.json`.
ROM SHA-256: `baa542304ec55bcfd2fecae7c594bb8e161275e49f6c3145886f2ab2f52f75be`.
The 65,536-byte image uses 49,496 ROM bytes, 3,084 data/BSS bytes, 964 workspace
bytes and a conservative 4,924-byte stack bound within the 24,320-byte reserve.
Its manifest identifies all source/specification/tool inputs and exact three
interrupt veneers. The frame-alignment audit covers service and both decode
functions; deliberate odd accesses are rejected.

The existing BootHush build is
`C:/Repos/.task-temp/boothush-pc-after-active-20261116-a/a/boothush-active.exe`,
SHA-256 `8860227d991a6c6d252815019c2458a1b0af3cae49306ce43e7fdaff5dd04e3b`.
This is the active-profile tool build with the paused x64 archive retained,
not a source-complete BootHush distribution. No other emulator or WSL ran.

| Same +32 X test | Prior image | Buffered image |
| --- | --- | --- |
| Edge 2,048: final X / omissions | 254 / 14 | 288 / 0 |
| Edge 2,048: maximum complete IPL7 span | 4,738 cycles | 2,308 cycles |
| Edge 4,096: final X / omissions | 288 / 0 | 288 / 0 |
| Edge 4,096: maximum complete IPL7 span | 4,738 cycles | 2,340 cycles |

These are BootHush observations, not physical timing. Trace/plain output and
complete frames match. Commands and complete witnesses:
`C:/Repos/.task-temp/mac-buffered-mask-20261116-f/report.json`.
Four production SCC boundaries preserve D0-D7, A0-A7, exception stack, saved
SR and return PC; 18 deliberately corrupted comparisons are rejected:
`C:/Repos/.task-temp/mac-buffered-preservation-20261116-f/report.json`.

Both the formerly failing edge-2,048 case and edge-4,096 case now pass all
eight natural shared-window states, all 175,104 canvas pixels and 48,896 white
EGA margin pixels, plus Mac cycle/memory/frame replay. Reports:
`C:/Repos/.task-temp/mac-buffered-parity-2048-20261116-f/report.json` and the
corresponding `mac-buffered-parity-4096-20261116-f/report.json`.
The paired 286 image remains the independently built native diagnostic from
`pc286-pc-after-session-20261116-a`; these tests do not promote its loader.

Twenty-nine rebuilt existing motion/inbox checks pass. The new FIFO/deferred
decode tests pass 12,132 assertions in each of O0/O2; they include every word,
wraparound, exact capacity, overflow, saturation, delayed hardware mismatch,
late-latch handling and cancellation/rebaseline. Logs and identities are in
`mac-buffered-host-tests-20261116-f.log` and
`mac-buffered-host-20261116-f/report.json` under `C:/Repos/.task-temp`.

The native excessive-burst test deliberately requests +5,000 X while held.
It observes queue peak 128 and 22 queue overruns, first loss reason 5, release
without a stuck drag, and a subsequent exact -16 X move with the released
window stationary. There are 43 total decoder losses in this stress case;
it is explicitly not lossless. Fault remains zero. Reproduction and captures:
`C:/Repos/.task-temp/mac-buffered-recovery-20261116-f/report.json`.

Earlier attempts remain outside Git: XOR-only arithmetic (4,682 cycles),
packed-only (4,874), split READY paths (4,262 at the slower interval), and
buffering with aggregate copies. The latter still called a byte-copy helper
for 1,558 cycles inside a 3,420-cycle SCC span, identified in
`mac-buffered-attribution-20261116-b.json`. Removing that copy and the obsolete
foreground inbox exclusion is the material architectural change. Failed
attempts are not passing qualifications; the new API is opt-in.

## Remaining paired work

The fast-input regression above is fixed at the tested rates. Production
286 media/installer work, the other existing release gates, physical/human
qualification, source/tool review and publication disposition remain separate.
This receipt does not declare a completed release or waive any such gate.
