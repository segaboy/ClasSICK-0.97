<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0214 - Bounded loop and register-transfer qualification

2026-11-18; SPEC-0123 / IMPL-0153. Corrected final qualification passes
596,298 checks in each release, sanitized and C99 host profile, 39 host
suite invocations, five native 286 cases and six native Mac cases. Four
mutants are rejected with distinct diagnostics. Native 286 exercises the
actual banked adapter, 128 KiB guest storage and 64-byte writes across three
banks; compact model receives object/import qualification only.

[Canonical receipt](../../docs/development/m68k-transfer-20261118.md) owns
exact inputs, native outcomes, replay scope, late-failure atomicity, no-runtime
checks and target budgets. It preserves the pre-fix passing suite and separate
failing alignment reproduction, then records the fix and new preservation
regression. Existing flat regressions pass 30 host/four 286/five Mac cases;
banked regressions pass 36 host/five 286 cases. These original bounded tests
do not establish full CPU, protected application, System startup or physical
hardware compatibility. Human provenance review remains separate.
