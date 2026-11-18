<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0210 - Startup header bounds and paired acquisition

2026-11-18; SPEC-0119 / IMPL-0149 / SRC-0617.
Original tests/mac-boot-header.c, probes/storage/mac-boot-header.[ch],
probes/mac128k/boot-header.c, probes/pc286/boot-header.c and
scripts/Test-MacBootHeader.py. Host Release and ASan/UBSan each pass 137032
assertion evaluations. Twelve actual-provider native runs pass across Mac68000
and286, including malformed metadata, second-sector atomic preservation,
fresh recovery and a detected name-bound defect. Exact pre/post identities,
commands and artifact results are in the retained b report and current c report indexed by the
[canonical receipt](../../docs/development/mac-boot-header-20261118.md).
Follow-on report d qualifies four native runs through explicit --disk using
only the generated original DC42. Six malformed-container controls refuse in c.
The explicit route has no original-header oracle, mutation or fixed fault.
All reports preserve their exact source hashes; c and d inputs remain unchanged.
All fixtures are independently generated; no selected disk or copied boot
payload. No historical startup, legacy execution or physical hardware claim.

Follow-on authorized selected-container trial was run by the coordinating lead,
not this implementer. The selected-a report records four successful native
runs, identical metadata across CPUs, exact repeats and unchanged source/input
identities. This implementer read only its numeric report. The canonical receipt
retains the sanitized scalar/digest observation separately from original fixture
qualification; no entry bytes, dispatch inference or System-startup claim.
