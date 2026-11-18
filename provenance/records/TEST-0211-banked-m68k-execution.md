<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0211 - Banked interpreter transactions and native regressions

2026-11-18; SPEC-0120 / IMPL-0150. Final `banked-m68k-20261118-d` passes
36 existing hosted sub-suites and five native286 cases. The original 128 KiB
fixture fetches, transfers control, reads/writes data and uses stack operations
across scattered banks. Guard-denial, final-write refusal, final-SP failure,
A-line policy, native envelope and lifetime controls preserve the required
state. Early-store, early-frame and missing-A-line-policy mutants are detected.

`banked-m68k-flat-20261118-a` passes 30 hosted sub-suites, C16 compile/import
audits, four native286 and five nativeMac outcomes. Updated primitive
`pc286-banked-guest-20261118-e` passes all five cases. Coordinator CMake builds
the startup/bus/guard targets and its selected CTest run passes 12/12. Exact
commands, hashes, replay inspection limits, preserved logs and distinction
between worker/coordinator results are in the
[canonical receipt](../../docs/development/banked-m68k-execution-20261118.md).
Only original inputs were executed. No System boot, protected-program,
physical-hardware or whole-runtime-performance result is claimed.
