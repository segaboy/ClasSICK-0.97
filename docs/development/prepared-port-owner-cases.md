<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Prepared-port owner cases (SPEC-0044) - UNEXECUTED PLAN

No case below has been executed, and none is a fixture or a result. Future implementations require matching semantics on native68000/x86; audit
rows require code/artifact evidence, not a fictional execution claim.

| # | Case | Expect |
|---|---|---|
| A1 | Nonzero header in loaded world before admission | Refuse; no claim, no g |
| A2 | Bad candidate (odd ID, overlapping records) | Pure refusal; zero effects |
| A3 | Cross-binding port overlap or exact share | Refuse pre-claim |
| A4 | Generation or epoch stale at admission | Refuse; world unchanged |
| A5 | Token push destination unmappable | Refuse at preflight; no g |
| A6 | Valid admission | (A5)=g, cell=0, frame published, claim consumed |
| A7 | Second admission, same generation | Refuse |
| A8 | Guest writes (A5) | Policy refusal (CPU stop) |
| A9 | Other 28 header bytes altered by trusted map | Quarantine |
| A10 | Default `cs_image_run`, rectangles: header==g | Refuse (still expects zero) |
| S1 | GetPort with cell=0 | d=0, A7=S+4, 1 instruction |
| S2 | SetPort(ID1), then GetPort | ID1 |
| S3 | Guest stores ID2 to g, then GetPort | ID2 |
| S4 | SetPort NIL, unknown, or odd | Inert refusal; no effects |
| S5 | Unknown ID in cell, then GetPort | Inert refusal |
| S6 | d aliases cell, reservation, record, stack or header | Refusal |
| S7 | Set: same cell read/write | Allowed |
| S8 | Logical output translates onto record/current physical bytes | Refuse before effects; invalid candidate before claim, admitted trusted map mutation quarantines |
| S9 | Scratch or parameter crosses root token | Refusal |
| S10 | Auto-pop, OS, or reserved word; patched slot | Refusal pre-effect |
| S11 | Registers and SR other than PC/A7 | Unchanged |
| S12 | Caller frame after any outcome | Unchanged |
| S13 | Fallible map lookup/callback during commit | Forbidden by construction; audit code when implemented |
| Q1 | View snapshot mutated mid-root | Quarantine; earlier effects kept |
| Q2 | (A5)!=g via trusted fault injection | Quarantine; actual frame kept |
| Q3 | Guest edits opaque record bytes | Not corruption |
| Q4 | Quiescence unprovable, then retire attempted | Quarantine; unresolved borrow/reservations remain; no cleanup or RELEASE |
| R1 | Clean stop retire | Below-A5 zeroed, anchor zeroed, RELEASE called |
| R2 | Guest-fault stop with arbitrary cell ID | Retire succeeds |
| R3 | Map failure during destination validation | Quarantine; nothing cleared |
| R4 | Handback/RELEASE fails after infallible canonical scrub | Counts/reservations/consumed tombstone retained; quarantine |
| R5 | Retire after quarantine | Refuse |
| R6 | Stack and stop snapshots after retire | Preserved |
| R7 | Native view pointer after proven-quiescent root | None; scalar cleanup still works (future code audit) |
| R8 | Rectangle lane: GetPort/SetPort words | Refuse (unchanged) |
| Q5 | Scalar profile tag forged without matching owner identity | Refuse before claim or quarantine if admitted; never authorizes g |
| Q6 | Caller cannot keep an unresolved borrow live | Refuse before claim |
| R9 | View storage ends after proven quiescent stop, then retire | No view dereference; validate owned scalar/map/header state |
| R10 | Header cleared but RELEASE fails; attempt second entry | Quarantine/reservation/tombstone retained; refuse entry |
| S14 | Nonselected binding/view/record extent changes | Full table checked; admitted trusted change quarantines |

Q5..S14 belong to the same unexecuted plan. No case, fixture, test or runtime probe was run in this intake.
