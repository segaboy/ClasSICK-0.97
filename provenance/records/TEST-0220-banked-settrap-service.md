<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0220 - Banked SetTrapAddress on an original guest workload

2026-11-19. SPEC-0129 / IMPL-0159. Host tests in `tests/banked-trap-set.c`
exercise all directory indexes and CCR inputs, failure visibility, aliases,
remapping and the old entry's narrower service admission. BootHush runs the
native `boot-banked-services.c` fixture with split banks, final-byte failure,
successful retry, retrieval and explicit guest JSR/RTS. Optimized and
unoptimized-wrapper builds pass. Four Set-specific mutants and three Get
mutants each trip their expected diagnostic. Evidence, tool identities and
reproduction appear in the [qualification receipt](../../docs/development/banked-settrap-service-20261119.md).
