<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0207 - Native286 bank access verification

2026-11-18; SPEC-0117 / IMPL-0147. Optimized/unoptimized native probes and
three rejected defects pass. Actual scattered physical RAM,128KiB logical
mapping, cross-bank access, boundary/permission/lifetime refusal and unchanged
outputs are checked with full loaded-payload and input-identity verification.
[Canonical receipt](../../docs/development/pc286-banked-guest-20261118.md).
No flat-API/interpreter integration, hardware qualification or stack high-water
measurement is implied.
