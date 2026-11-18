<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0121: native read-only resource metadata preflight

2026-11-18. Original workload composed from SPEC-0115 MFS access and SPEC-0116
resource transport. It admits no new historical format fact, protected source,
executable loading, Resource Manager trap, System boot or physical hardware claim.

## Workload

Mount the supplied stable read-only block device. Enumerate every directory entry
through the strict MFS iterator. For every nonempty resource fork, bind the MFS
transport and open its resource map. Binding validates its whole allocation chain;
open validates the entire admitted resource metadata profile. Empty logical forks
are skipped. There is no payload extraction or interpretation, resource lookup,
name export or media write. Permitted bounded metadata cache read-ahead remains
the SPEC-0116 behavior.

Stop at the first failure. File ordinal is one-based directory enumeration order;
a mount failure uses ordinal zero. Only completed resource opens contribute to
aggregate type/reference counts and warning masks. Files and nonempty forks seen
include the entry that fails. Check aggregate addition before publication. On open
failure verify the entire destination view retains its prior canary bytes; an
atomicity violation has its own probe outcome. The report contains no file name,
type key, resource ID, payload byte or payload digest.

## Diagnostic contract

The shared probe owns 16 volatile uint32 words. Native target wrappers publish
the returned outcome in word 14 and then a fixed terminal marker B8, including
classified refusal, so an optional authorized runtime disk can yield bounded
failure diagnostics without waiting for a success-only stop condition.

| Word | Meaning |
|---|---|
| 0 | Magic `52465031` (`RFP1`) |
| 1 | Stage: 1 mount, 2 directory, 3 bind, 4 resource open, 5 completed |
| 2 | Directory files seen |
| 3 | Nonempty resource forks seen |
| 4 | Resource forks opened successfully |
| 5 | Type records in completed opens |
| 6 | Resource reference records in completed opens |
| 7 | Warning mask OR across completed opens |
| 8 | First failing file ordinal, zero on success or mount failure |
| 9 | Status domain: 0 none, 1 MFS, 2 resource transport, 3 probe arithmetic |
| 10 | Failure status in that domain, zero on success |
| 11 | Resource field on open failure, otherwise zero |
| 12 | Resource absolute position on open failure, otherwise zero |
| 13 | Header file count after successful mount |
| 14 | Outcome: B8 complete; D1 mount, D2 directory, D3 bind, D4 open refusal; DE aggregate overflow; DF failed-open destination changed |
| 15 | Reserved zero |

## Paired verification

Use a separately authored original 400 KiB MFS fixture with fragmented resource
forks and a resource map beyond 64 KiB. Its declarative oracle supplies aggregate
counts and one exact metadata-sector failure. Execute baseline, injected failure
and fresh recovery through both the native 286 modeled floppy provider and the
native 68000 modeled IWM provider. Preserve provider-specific fault receipts.
Compare complete reports across architectures and fresh runs, inspect all loaded
286 program bytes, and check all input hashes before and after the run.
The admitted emulator permits inspection of RAM, not Mac ROM addresses. Mac ROM
artifact identity is pinned before and after execution; do not call that a loaded
ROM byte comparison.

The optional explicit DC42 route is an authorized runtime input only. It gets no
original-fixture expected counts and no fixed-sector injection. Repeat it cold;
report actual completion or classified refusal, separately from harness validity.
Do not inspect/export payload bytes or add private media to the repository. The
existing admitted DC42 container helper supplies logical sectors; pin that helper
alongside every source, compiler/tool closure and exact emulator identity.

Keep the existing Mac reset/vector and low-memory budget when it fits. Any larger
isolated probe layout must state its data/BSS, stack and framebuffer boundaries
explicitly and enforce them at link/verification time. Report actual code, BSS,
local compiler frame sizes and modeled cycles. Local frames are not cumulative
stack proof; modeled device observations are not physical performance evidence.
