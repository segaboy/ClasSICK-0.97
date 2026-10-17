<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0043 - Pure native drawing-port selection values

Version1, accepted2026-10-17; SRC-0258..0261 / ADR-0049. Owner "get started"
accepts the proposed shared native selection core, independent tests and paired
compiler verification. This is original engineering derived from SPEC-0042's
selection rules, not new historical evidence. No guest binding, target trial or
release. SPEC-0041 and existing workloads/ROM/EFI inputs stay unchanged.

## Frozen API and lifetime
`personality/system1/quickdraw-select.h` is the normative native API. The core
implements get/set over a caller-owned `cs_qd_selection`, a raw32 current value
supplied afresh each call, and an output plan. There is no initialized manager,
cache, mutation of current state, retained pointer, allocation, callback, libc,
mutable global, CPU/frame access, guest RAM access or drawing. The same C builds
for native68000 and x86. A plan is only {value, borrowed view}; it does not promise
commit eligibility at a guest boundary. Caller applies native selection, if any.

Selection, output plan, each active view and each view's port have truthful live
native storage for this synchronous call and no concurrent mutation. Successful
plan.view is borrowed only while the caller continues to keep the validated
selection/binding/view/backing alive and unchanged; any change requires a new
call. Arbitrary dangling/unmapped native pointers cannot be diagnosed by C and
are outside the API. Null, misaligned and overlapping addresses into live storage
are supported refusal inputs: check before dereferencing the corresponding type.
No integer guest ID is converted into a native pointer. No guest pointer flags
are stripped. Output padding is unspecified on success; input padding is ignored.

## Complete validation on either operation
Require selection/plan nonnull, natural C alignment, representable uintptr_t
half-open spans and no overlap between their full sizeof objects. count is1..2;
unused binding slots are ignored (never dereferenced). globals_begin, a5 and g
are even strict24 values (<=0xFFFFFF); globals_begin<a5. The engineering206-byte
reservation [g-202,g+4) must fit wholly in [globals_begin,a5), checked without
unsigned under/overflow. These are numeric values, never guest reads.

Each active ID is nonzero, even, strict24. Its108-byte record is wholly below A5
and at/above globals_begin, disjoint from the206 reservation and any other record.
Adjacent half-open ranges are accepted. All active views and their ports are
nonnull, naturally aligned and have representable uintptr_t sizeof spans. Every
pair among selection, output plan, active views and active ports must be disjoint,
including exact sharing. Check view spans before reading view fields and all port
spans before output writes. No port contents are read or changed. A view is valid
iff x,y are in [-32768,32767], top/bottom in[0,64], left/right in[0,128]. Independent
empty/inverted clips are valid. View fields are not reinterpreted as guest records.
Unknown controls, reports and guest map backings are not inputs here: disjointness
from those and admission integrity remain future SPEC-0042 owner prerequisites.

Validate the WHOLE table on every get/set, including the nonselected binding.
Then current must be0 or exactly one registered ID; nonzero current's full binding
is therefore revalidated even on set. get succeeds for0 with {0,NULL}, otherwise
{current,current's view}. set additionally requires requested nonzero registered
ID and succeeds with {requested,requested's view}. No fallback/cache. Same-value
set is successful. Native argument validation failures, including changed/truncated
descriptors, return0. Neither function quarantines or authenticates a root.

## Effects and owner classification
Preflight all validation before writing either output field. Return1 on success,
writing only plan.value and plan.view. Return0 leaves all input/output bytes,
views and backing unchanged. No guarantee of error precedence is exposed. Pure
core refusal is distinct from an ADMITTED owner detecting trusted capability,
record/map/lifetime or binding corruption: the future SPEC-0042 owner must
quarantine such corruption, preserve prior effects and actual stopped frame.
Malformed ordinary guest service arguments remain pre-effect service refusals.
This resolves C22b's wording; it does not implement owner admission or quarantine.

## Verification and limits
Independent API-only tests must check two-way switching, NIL get, current supplied
afresh, complete-table rejection, boundary/underflow/overflow/adjacency, view limits,
alias/alignment refusals and byte-for-byte no-effect failures. Oracle author has
no implementation/session history. Compile strict freestanding on both native
architectures, verify no undefined imports, and run appropriate hosted/sanitized
tests. Cross-compilation is not a Macintosh boot or target test. SPEC-0042 cases
involving CPU, BE stores, current-cell writes, lifetime epochs, quarantine, traps
and guest commit are still unexecuted plans; no test here closes them. P1/P2/P4
contracts and the guest selection-only binding/workload remain later work.
