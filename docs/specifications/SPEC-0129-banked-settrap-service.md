<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0129 - Prepared banked SetTrapAddress completion

2026-11-18; IMPL-0159 / TEST-0220. Original integration of already admitted
SPEC-0020 trap-address semantics, SPEC-0025 collapsed completion and the
SPEC-0124 native envelope/guard contract. No new historical interface claim,
protected program, third-party implementation or selected media is consumed.

## Separate opt-in entry

`cs_s1_banked_trap_step` accepts A047 SetTrapAddress and the existing A146/A046
GetTrapAddress forms. `cs_s1_banked_step` remains Get-only, including its refusal
of A047. Existing bus/transfer entries still refuse every A-line. A147, other
flag variants, Toolbox words and arbitrary remapped intrinsics are refused.
Non-A-lines delegate with the same explicit repeated guarded fetch as SPEC-0124.

The full SPEC-0124 admission applies: disjoint, aligned live native descriptor,
directory, frame/report, limits, guard and complete retained contexts; stable
exclusive ordinary RAM and callbacks; no reentry; pure synchronous guards;
strict24, even PC/stack, S=1/T=0/I=7; no active manager; six writable scratch
bytes below unchanged A7, at or above 0x400. The native286 adapter excludes the
whole bank arena from the full CS/DS/SS envelopes. Neither entry discovers
hidden pointers, proves object validity or supplies concurrent publication.

## Preparation and publication

After routing/instruction guards and scratch preflight, resolve current native
slot 0x47 and require its target to equal the configured Set token. An odd or
invalid current slot is a service-state failure; an executable remapped target
is a policy refusal. Do not invoke generic trap dispatch after checking this
signature. Get retains its independent slot 0x46/Get-token check.

`cs_trap_check_set` is pure and shares validation with `cs_trap_set`: null
gateway returns ARGUMENT; malformed state/configuration returns STATE; a low
word index outside 0..511 returns NUMBER; an odd or unconfigured destination
returns ADDRESS. This ordering is unchanged. The destination must be an even
address in the existing configured ROM64K or RAM execution window; addresses
are logical tokens, never native callbacks. The validator does not inspect the
old destination entry. Thus a valid Set can repair a malformed old entry,
provided the dispatch slot 0x47 itself still resolves to the Set token.

Prepare only an index from D0's low word and replacement address from A0.
Prepare the completed frame without changing D0/A0 or other registers/A7:
SPEC-0020 successful CCR tests low D0, clears V/C and preserves X/high SR;
advance PC by two. The current slot target was resolved before preparation.
Set may replace itself, Get, or any other directory entry. The current call
completes using its previously resolved target; later calls see the new entry.
Repair of a remapped Set dispatch slot requires the trusted native owner, since
this profile does not expose direct-token calls or guest patch execution.

Validate continuation and obtain COMMIT permission. Stage completed SR and PC
as six big-endian bytes. Make exactly one atomic `write_bytes` call for all six
bytes. If it refuses, RAM, directory and frame are unchanged. After success,
publish the one prevalidated directory assignment and then the frame. Those
two native publications cannot fail under the exclusive lifetime contract;
there is no intervening callback, validation or allocation. Two fallible
scalar stores, an early `cs_trap_set`, or an early frame copy violate this rule.

Report/error rules match SPEC-0124. Unsafe native-envelope refusal leaves even
the report unchanged; other failures may update the report only. Service-called
means the named signature reached preparation, not that effects committed.
Malformed replacement/index failures are SERVICE with the precise trap status;
remapped/unsupported words or guard refusals are POLICY; backend failures are
ADDRESS plus memory status. Get behavior and the generic native trap APIs
remain compatible.

## Evidence and limits

Original host fixtures cover all 512 indexes and CCR inputs, every invalid low
word index, destination limits/odd addresses, pure validation equivalence and
malformed-entry repair, signature remaps, self-patching, restoration, old Get
refusal, native aliases/alignment, guards, continuation and failed publication.
The native286 fixture executes an original Set/Get sequence using a 128KiB
banked region and six scratch bytes crossing three physical banks. Last-byte
failure must preserve the entire native directory, frame and scratch; retry
must succeed. Deliberate early-directory, early-frame, split-store and omitted
Set-signature variants must produce distinct failure markers.

The Windows-native harness captures source/spec/helper and selected tool
identities before its first compile and verifies unchanged inputs afterward.
It runs the old Get fixture and generic trap regressions with the new source.
Complete loaded native payload bytes are compared over identical-state fresh
replays under the CLI inspection budget, and the original loader checks the
whole payload on each replay. This is not a simultaneous complete RAM snapshot
or cryptographic authenticity proof. No runtime libraries are linked.

This adds one prepared native directory effect to the existing six-byte RAM
transaction. It does not implement the historical low-memory trap table,
arbitrary guest patches, active memory-manager transactions, general exceptions,
Toolbox, callbacks, executable loading, selected System startup or physical
hardware fidelity. Existing RAM patch windows remain limited to 64KiB.
