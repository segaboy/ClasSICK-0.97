<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Input cost and post-interaction observation follow-up

Later continuation: [buffered input and paired qualification](paired-preview-readiness-20261116.md)
fixes the fast-input regression at the stated modeled rates. This earlier
receipt and its failed measurements remain historical evidence.

2026-11-16, continuing from `837be60`; SPEC-0097 / SRC-0598 / IMPL-0113 /
TEST-0189. Codex independently implemented and self-reviewed the changes.
No game, Apple/Microsoft or third-party emulator implementation informed the
ClasSICK code. Unrelated working-tree changes and research originals remain.

`inbox_axis` now returns the other operand directly when either operand is
zero. This implements the already specified sum without the general checked
addition. Metadata, nonzero-vector, button, reversal, overflow and fallback
rules remain. Five rebuilt focused inbox tests pass, including the existing
arithmetic oracle, metadata, interleaving, integration and freestanding checks.
Build log: `C:/Repos/.task-temp/classick-zero-axis-host-build-20261116-a.log`.
CTest used `classick-inbox-single-validation-host-20261115-c` and regex
`^window-inbox\.`. This is self-verification, not independent review.

Windows-native Mac twins and all 49 source/tool inputs are identified in
`C:/Repos/.task-temp/mac-zero-axis-20261116-a/manifest.json`. ROM SHA-256:
`8c89b8f6f1607247c856eb8c7ee8fd4a342e96a8f09c9004ed7a62c4b082c2ce`.
The existing window/FAST/SHORT/loss-count/invert-3 profile keeps full-register
interrupt veneers. ROM use is 47,984 bytes (52 more), data/BSS 488, workspace
32,768..33,732, and conservative stack bound 4,720 within the 24,320 reserve.

## Measured result

Same +32 X request at cycle 100,000,000, stop 105,000,000, invert 3,
guard 4,096, trace range 99,999,000..100,300,000:

| Edge interval | Previous maximum complete IPL7 span | Revised | Final X | Omissions |
| --- | --- | --- | --- | --- |
| 2,048 | 4,782 | 4,738 | 254 (expected 288) | 14 |
| 4,096 | 4,780 | 4,738 | 288 | 0 |

This small reduction does not fix fast motion. The previous full-save READY
comparison is in [the native toolchain receipt](windows-m68k-toolchain-20261114.md).
Every traced/plain pair matches complete JSON and frames, with undropped
selected ranges. Exact commands and results are in
`C:/Repos/.task-temp/classick-zero-axis-mask-20261116-a/report.json` and its
`-b` sibling. The latter uses the new BootHush build below; both have the same
measured outcomes. These are modeled boundaries, not physical timing.

## Shared tool and paired regression

BootHush's new `--until-pc-after` selects a later visit to a known instruction
without modifying guest code. Its canonical API/build receipt is
`C:/Repos/boothush/docs/pc-stop-cycle.md`. Active-profile twin SHA-256:
`8860227d991a6c6d252815019c2458a1b0af3cae49306ce43e7fdaff5dd04e3b`.
Fifty-eight focused tests pass; two optional fixture cases skip. The paused
x64 archive is linked unchanged, so this is not a complete distribution build.

On that executable, a fresh native 286 diagnostic passes eight shared-window
states/full-EGA checks and two pixel-mutant controls. Its payload remains
`bb0f402bbf7b9d22d7999f7afc6e44f8defdc74de4acc012af2f349818dfd336`,
and diagnostic disk remains
`a01bb95b6a3840a0a5a889951d75c421bb4823b1189a8cfae1e17d7b4089350e`.
Exact invocation and results:
`C:/Repos/.task-temp/pc286-pc-after-session-20261116-a/report.json`.
The revised Mac image matches eight states, all 175,104 canvas pixels,
48,896 white margin pixels and complete Mac replay at edge/guard 4,096:
`C:/Repos/.task-temp/mac286-pc-after-4096-20261116-a/report.json`.
The preceding same-image comparison on the prior emulator is retained in
`mac286-zero-axis-4096-20261116-a/report.json` under the same scratch root.
This remains a diagnostic disk and model comparison, not production media.

## Port feedback and next dependencies

The port implements a source-informed jump out of late side-attack recovery
for both players. Its `docs/recovery-jump-evidence.md` records 34 functional
scenarios, deliberate threshold controls, ten original call sites, and two
native movement calls after both jumps. The later-boundary checks preserve
registers, stack and frames and match unobserved controls. The transfer to
ClasSICK is this diagnostic method and BootHush API; game-derived code and
behavior are not ClasSICK implementation inputs or hardware proof.

Fast Mac input, production 286 boot/install, remaining target/input and
human/physical qualification, source/tool/provenance review and separately
approved publication remain open. No release candidate or completed release
is asserted. No WSL, external AI service, other emulator or desktop input ran.

## Retained continuation snapshot

After implementation commit `3482a3cf4dc23c1a3d6210ed91f48a52817d2b4f`, the
existing guarded preparer validated the current source fingerprints and made
`C:/Repos/.task-temp/classick-paired-engineering-snapshot-20261116-c`.
It contains the exact Mac ROM and diagnostic disk, build/test records,
original frames, instructions, checksums and explicit unqualified markers.
Its two documented readiness commands reach Mac stage 16 at 60,901,050 cycles
and 286 POST b0 at 7,509,810 cycles. Prior snapshots remain unchanged.

Related local revisions are BootHush
`76861e7b51bcf1fee8bf0cc9360abc02545ef4d8` and port
`443ec9aaac74e08cf9b1b6fa679b07b514846b60`. The repository guard passes 1,820
indexed text/source files, 134 registered PNGs and two reviewed GIFs. This
does not establish independent provenance review. CLI access to GitHub failed
because `github.com` could not be resolved; remote publication is unverified.
