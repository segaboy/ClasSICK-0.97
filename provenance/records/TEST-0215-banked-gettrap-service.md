<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0215 - Guarded banked service transactions

2026-11-18; SPEC-0124 / IMPL-0154. Final `banked-services-20261118-c` passes
619707 assertions in each Release/sanitized/C99 host profile, plus optimized
and unoptimized native286 baselines and three deliberate native defects.
Original fixtures test both Get words, index/CCR/register behavior, remapped
directory refusal, first/second guarded-fetch denial, native aliases/alignment,
continuation, last-byte failure and unchanged frame/directory/RAM. An actual
128 KiB banked program commits its six-byte service scratch across three banks.
Early-frame, split-write and omitted-signature mutations reach distinct markers.
Coordinator CMake/CTest also passes 14/14 existing and new CPU/service cases;
the immutable saved log and its verified hash are cited in the receipt.

The [canonical receipt](../../docs/development/banked-gettrap-service-20261118.md)
records exact source/tool/report hashes, full-payload replay verification,
independent reviews and earlier pre-dependency-fix runs. No protected program,
selected System boot, physical hardware or whole-runtime performance result.
