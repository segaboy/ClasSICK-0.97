<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0154 - Opt-in banked GetTrapAddress completion

2026-11-18; SPEC-0124 / TEST-0215. Original backend-neutral service wrapper
reuses the admitted SPEC-0020 GetTrapAddress interface and SPEC-0025 collapsed
completion through SPEC-0123's atomic transfer callback. A146/A046 require
current slot46==get_token; only read-only decode/get primitives run. Six scratch
bytes commit atomically after validation and COMMIT permission, before frame
publication. Existing guarded APIs still refuse A-lines; other words in the
new wrapper delegate or refuse according to the explicit contract.

GPL-3.0-or-later; no protected or third-party implementation input. The
[canonical receipt](../../docs/development/banked-gettrap-service-20261118.md)
owns exact identities, code/harness reviews, native tests, the corrected shared
dependency rerun and limitations. No Set, memory manager, selected-disk/System
startup, Mac-native integration or physical qualification is claimed.
