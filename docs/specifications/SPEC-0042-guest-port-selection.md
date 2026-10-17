<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0042 - Guest port selection lane (GetPort/SetPort), phase1

Version1 accepted2026-10-17 as a contract only; SRC-0244..0248 / ADR-0048.
Profile `guest-port-select-v1`:
original engineering, not general QuickDraw or January1984 parity. This document
authorizes no implementation, fixture, test, registry entry, build, trial or release.
Labels: [F] independently reviewed later-edition published fact;
[E] engineering choice; [X] needs an amendment to an existing contract (not made here).

## 1 Scope and non-goals
Phase1 is PREPARED selection only: canonical GetPort and SetPort in a NEW selected-port
lane. Not in scope: InitGraf/OpenPort/InitPort/ClosePort, global manager installation,
default cursor/pattern assets, 206-byte globals initialization, regions, port
creation/lifetime management, guest drawing. SPEC-0036 `cs_image_run_rectangles` and
ordinary `cs_image_run` stay byte-for-byte unchanged. GetPort/SetPort NEVER coexist with
the fixed-port PaintRect lane while PaintRect ignores current-port state. In the new lane
A8A2/A8A3 and every other A-line refuse until a separately reviewed adapter from actual
guest record state to a SPEC-0041 view exists. SPEC-0041 stays unchanged.

## 2 Sources and assumptions
- [F] SRC-0071 (1985) I-165: SetPort makes the supplied GrafPtr current; GetPort(VAR)
  writes the current pointer; the current pointer is thePort. OpenPort/InitPort also
  change it (deferred). I-162: InitGraf's first current pointer is NIL.
- [F] I-163, II-20: (A5) holds guest address g of thePort, not the GrafPtr itself;
  public globals extend downward below g. I-209: grafSize 206. No private bytes rebuilt.
- [F] SRC-0068 QD-36 agrees on Get/Set and warns ports should be opened.
- [F] Appendix C III-217/220: GetPort A874 / SetPort A873, images checked.
  I-165 signatures plus the reviewed SPEC-0029 Pascal pointer convention give P=4.
- [E] Opaque record extent108 for this profile (D-04); not a recovered layout or
  verified January1984 size. The106 alternative and full field offsets stay open.
- [E] Everything else here. Later editions do not prove January1984 behavior. No manual
  sample contributes to this contract; no standalone equate file, Apple/MS implementation,
  disassembly or emulator source used. Published interface constants remain eligible facts.

## 3 Native and legacy paths
One original pure selection core (validation values and commit plans) is shared by
native68000 and x86 Mac/PC builds with the same SPEC-0041 view bindings. PC OS services
execute natively. The 68000 interpreter remains only the legacy application boundary:
the owner coordinator serializes, preflights map access and executes the plan. That is a
guest boundary, not an interpreted OS implementation.

## 4 Graphics capability (prepared, owner-bound)
A caller-prepared immutable capability, bound to one image root. Contents:
- source image generation and execution generation, manager/zone epoch;
- map, lifecycle and owned layout identities (globals_begin, A5, below/above sizes,
  stack base/top), plus the expected g and the expected (A5) header value;
- 1..2 bindings {guest port ID, record extent, SPEC-0041 view}; the extent is the
  profile record size from D-04;
- trusted binding metadata digest, checked at admission and at every service.
IDs: unique, nonzero, even, strict24, inside declared below-A5 application storage,
never stack, CODE, jump table, manager metadata or the g reservation. No host address
or function pointer ever enters guest state; the guest sees only the raw ID.
Records are opaque interface storage in phase1. Get/Set select identity only; no
pixel, bitmap, portRect or layout semantics; unregistered ports are never accepted.
Generation/epoch mismatch, teardown or retirement revokes it. No raw guest view or
native pointer is kept past the root.

## 5 Globals anchor g and current cell
- g is immutable and prepared for this root. [E] Reservation is the 206 bytes
  [g-202, g+4), fully inside the CODE0-declared below-A5 space; no implicit size
  increase and no launch refusal is added to other profiles. (Offset choice [E], not
  a fact.)
- Native setup, inactive only, writes g into (A5)..(A5)+3 before root admission. All
  other 28 application-parameter bytes keep their existing checks.
- g..g+3 is the authoritative CURRENT guest port ID. Prepared setup starts at0;
  later services/direct stores may select a registered ID. NIL after InitGraf is [F],
  but this prepared profile does not claim to implement InitGraf. Remaining reservation bytes are untouched and not initialized.
- Each service re-reads (A5) and checks it equals g, then re-reads the current cell.
  Nothing is cached. A valid direct guest store of a registered ID to g is honored.
  Unknown current IDs refuse; no fallback to a previous view.
  For either call, nonzero current requires its full registered record extent and
  live native binding to be revalidated before effects, even when GetPort only
  returns the ID. SetPort also validates its proposed new record independently.
- Guest writes to (A5) remain refused (above-A5 is read-only under existing policy).
  InitGraf changing that slot mid-run is deferred.
- [X] Integrity: the capability authenticates expected g, generation and header.
  Existing header/owner checks are not disabled, the old dirty flag is not widened, no
  protected image is mutated temporarily, no fake context is borrowed. Default and
  fixed-port modes still need their original zero header and behavior.

## 6 Bindings and native backing
Bindings, metadata, views and native controls are truthful, live, aligned and immutable
for the root. Each binding has its own view and disjoint1032-byte native port backing (D-06).
Exact sharing and partial overlap both refuse; future sharing needs separate review.
Any alias between views, ports, capability, controls, reports, guest map backings
or guest data refuses. Phase1 has no Memory Manager, allocator, callback,
guest patch or runtime reentry. The image's existing exclusive execution participant
covers the capability and its views, so reset, compaction or destruction cannot happen
during execution. Only current contracts are referenced; private heap state is never
reconstructed.

## 7 State machine
PREPARED (inactive, validated) -> INSTALLED (g in (A5), current=0) ->
ADMITTED (root entry; existing states 1/2) -> STOPPED (3/4) | QUARANTINED (5) ->
RETIRED (below-A5 scrub zeroes the current cell; capability revoked).
Setup or capability change while active is refused. Reentry is refused. A consumed
generation is never resumed. Retire and quarantine keep the existing paths.

## 8 Calls and frame
Two returning Toolbox stack procedures, P=4, no function result, canonical words only
[F words; E admission policy]. At the trapped A-line, S=A7.
- SetPort(port: GrafPtr): raw32 scalar at S. It must be a registered ID whose full live
  extent is checked; then only g..g+3 is written. NIL refuses [E] (no historical
  behavior claimed).
- GetPort(VAR port): raw32 address d at S. Writes the raw current ID (0 included) as 4
  big-endian bytes to d..d+3, never a native pointer. Phase1 d is even and inside
  declared below-A5 application bytes [E]. Stack-local outputs, heap objects and aliases
  are deferred explicitly; this is not a universal API rule.
- Both: 6-byte scratch S-6..S-1 and parameter S..S+3 lie in the current owned stack
  below the root token (S+4 <= root). Continuation PC+2 is strict, even, an owned
  opcode. Proposed frame A7=S+4, PC+2; other D/A registers and SR are preserved as an
  engineering superset. SR, A5 and supervisor checks match SPEC-0036.
- Auto-pop (bit10), bit9/reserved, OS-format, unknown words and modified gateway slots
  refuse. Selected directory slots must read unsupported at admission and at each call.
  No global builtin identities, GetTrapAddress or direct-JSR equivalence.

## 9 Alias and range rules
| Object | Must lie in | Must not overlap |
|---|---|---|
| parameter S..S+3 | owned stack, below root token | root token, scratch |
| scratch S-6..S-1 | owned stack | parameter, port records, g reservation |
| SetPort ID record | below-A5 app bytes, full extent | g reservation, other record, A5 |
| GetPort dest d..d+3 | below-A5 app bytes, even | g reservation, any record, A5/header, stack |
| current cell g..g+3 | inside 206 reservation | any record, dest |
| native views/ports | host, disjoint | capability, controls, reports, map backings |
Read/read aliases between records and dest refuse too (as in SPEC-0029).

## 10 Environment span and commit
[X] SPEC-0029's pure handler and 16-byte staged stream do NOT authorize SetPort's
hidden output or persistent manager state. A dedicated owner-coordinator environment
span declares: the (A5) seal read, the current-cell read/write, and the GetPort output.
No generic handler gets hidden guest memory access. No retained-pointer descriptor
poses as a pure span. Order: (1) capability/generation/epoch/integrity; (2) word and
directory; (3) frame, stack, scratch and continuation; (4) parameter read; (5) seal
and current-cell read and validation; (6) target/dest map, range, alignment, alias and
backing; (7) stage values and frame. Every read/write span and translated backing
(including implicit current-cell output and scratch) is checked with overflow-safe
half-open arithmetic under the exclusive lease. No fallible access, callback,
allocation or revalidation occurs during commit. Commit is total and deterministic: scratch SR/PC,
then the current cell (Set) or dest (Get), then frame. Failed preflight changes nothing
except diagnostics/workspace. A success is one instruction. Parent stop, budget,
retire, caller frame and report semantics follow SPEC-0036.

## 11 Refusal classes
- Service refusal (pre-effect, CPU/policy stop, not quarantine): unregistered, odd,
  high or NIL ID; unknown current ID; bad or aliased dest; frame/stack/scratch
  violation; unsupported word; patched slot.
- Owner quarantine: capability digest, binding, view or native-control change;
  (A5)!=g; header mismatch; generation/epoch/identity mismatch; map/backing alias.
  Prior effects and the actual stopped frame are kept.

## 12 Prerequisites needing existing-contract changes [X]
P1 SPEC-0036/code-run integrity: the (A5) header is assumed zero. A capability-specific
expected-g clause is needed. P2 SPEC-0029: an explicit owner-coordinator environment-span class.
P3 cross-binding disjointness must be validated; SPEC-0041 is unchanged. P4 owner lifecycle: participant coverage for capability/views. P5 trap
registry: canonical words are source-verified, but no admission/registry binding exists. P6 CODE0 below-A5 declaration covering the g
reservation and records. None of these is amended here.

## 13 Future original workload (planned, no fixture now)
One independently authored selection-only program: GetPort -> expect 0; SetPort(ID1);
GetPort -> ID1; a direct guest store of ID2 to g; GetPort -> ID2; return a D0 code
derived from the results. No drawing, no pixels. Same result on native68000 and x86.
No manual sample is used.

## 14 Decisions and implementation order
[Lead decisions](../development/guest-port-lead-review.md) and
[unexecuted cases](../development/guest-port-cases.md) are normative profile inputs.
The first future implementation slice is the pure shared native selection core and
independent tests on68000/x86, after owner acceptance. It cannot mutate guest RAM or
enable traps. Next specify and verify P1/P2/P4 together, then the guest selection-only
binding/workload. A real guest-record drawing adapter, InitGraf/OpenPort and full
globals/assets/regions remain later work. No implementation permission follows here.

## 15 Native implementation slice and C22b clarification (2026-10-17)
The owner subsequently accepted SPEC-0043's pure native value/selection core and
independent tests. The original no-implementation statement above describes this
contract's original intake, not a prohibition on that separately accepted slice.
Guest binding/owner amendments remain unimplemented and unauthorized here.
Inactive/pure bad supplied descriptors return inert validation refusal; corruption
of an admitted trusted record/map/lifetime/binding is owner quarantine under§11.
C22b is clarified accordingly; no raw native pointer liveness probe is promised.
