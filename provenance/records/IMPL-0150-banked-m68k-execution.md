<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0150 - Native286 bank-backed guarded execution

2026-11-18; SPEC-0120 / TEST-0211. Original engineering integration of the
SPEC-0117 provider with the shared guarded interpreter, including SPEC-0118
instructions. A separate backend entry performs numeric span/read operations
and one final atomic scalar write before publishing the CPU frame. Existing
flat APIs retain their behavior. A-lines remain refused. The native adapter
owns inline binding/generation state and excludes the entire RAM arena from
CS and DS/SS native envelopes. No protected or third-party implementation was
used. GPL-3.0-or-later.

The [canonical receipt](../../docs/development/banked-m68k-execution-20261118.md)
owns exact source/report identities, independent code/harness review,
qualification, retained attempts and limitations. This closes one real native
bank-backed interpreter route, not manager migration, service dispatch,
selected-disk/System startup or physical qualification.
