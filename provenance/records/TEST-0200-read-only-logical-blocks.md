<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0200 - Bounded logical block reads

2026-11-17; SPEC-0111 / IMPL-0141. Windows Release and ASan/UBSan profiles each
pass 1032994 assertion/check evaluations from original generated RAM providers:
counts 1..63, high LBAs, every failure position, partial provider effects,
status mapping, preflight call suppression, bounds, output preservation and
canaries. These are check counts, not independent tests or reference observations.
The isolated CMake Release target and `storage.block-read` CTest also pass (1/1),
retaining this fixture in routine hosted coverage.
Native 68000 O0/Os and C16 small/compact compile/import checks pass. The actual
shared fixture passes natively in BootHush on both Mac128K (O0/Os) and 286,
and two mutated core implementations are rejected at their intended failure
markers on both CPUs under the same current emulator identity. Mac own-reset
fault fields remain zero. Complete loaded 286 payloads match and original
generated input ROMs/media remain unchanged. Earlier 286 observations under a
different executable are retained separately.
No MFS, physical disk, System boot, full segmented integration or physical
hardware claim. Implementation and tests share an AI author; independent human
review remains separate. Exact reports/commands and retained tooling correction
are in the [canonical receipt](../../docs/development/block-read-foundation-20261117.md).
