<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0159 - Banked SetTrapAddress completion

2026-11-19. SPEC-0129 / TEST-0220. The existing approved interface in SPEC-0020
supplies the directory semantics. A new opt-in entry in
`personality/system1/banked-services.c` shares the existing Get pipeline and
uses pure validation in `core/services/traps.c` to prepare the directory write
before committing guest RAM. Successful calls publish one native entry and
then the register frame. The earlier Get-only API retains its narrower contract.

Codex integrated and tested original project C, Python and synthetic input
against the approved specifications. No third-party code or assets added.
Human provenance review remains open. The [qualification receipt](../../docs/development/banked-settrap-service-20261119.md)
records the direct guest call and limitations.
