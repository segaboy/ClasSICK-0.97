<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0044 - Prepared-port owner boundary (SPEC-0042 P1/P2/P4)

Finalized engineering v1,2026-10-17; owner continuation SRC-0262..0265 / ADR-0050.
Human provenance review remains open; no implementation is authorized.
This is original engineering and a contract only. It contains no implementation,
fixture, API header, trap binding, build, trial or release. It makes no historical
claim. Profile `prepared-port-owner-v1` is opt-in and prospective. It does not
weaken any default rule. [E] marks engineering choices. [D] marks an explicit
disposition of an earlier clause.

## 1 Effect on existing contracts
- SPEC-0029: generic pure handlers, the 16-byte staged stream and the refusal of
  retained pointers stay unchanged. The span class in section 4 belongs only to
  the owner coordinator and is never a handler descriptor.
- SPEC-0034/0035/0036: default `cs_image_run` and `cs_image_run_rectangles` stay
  byte-for-byte unchanged. Their zero header, protected-field checks, admission
  order and retirement remain in force. Neither route gains GetPort/SetPort.
- SPEC-0042: P1, P2 and P4 become specified, not implemented. P3 is covered by
  section 5. P5 (trap registry) and P6 (CODE0 declaration) remain open.
- SPEC-0043: the pure core is unchanged. The owner calls it with values it has
  just read itself and never with cached values.
- [D] SPEC-0042 sections 5 and 7: "native setup writes g before root admission"
  and the INSTALLED state are superseded for this profile by the single admission
  transaction in section 3. INSTALLED is an internal transient. It is never a
  public, observable or reusable state, and it has no cancel or re-prepare API.
- [D] SPEC-0042 section 6, "any alias with guest map backings refuses": the
  native-view/port/control versus guest-backing prohibition stays absolute. It
  is NOT relaxed by current-cell RMW: that is overlap within one GUEST logical
  role. Read/write descriptors of the same declared guest bytes keep all existing
  truthful-map requirements; no new permission for distinct-region aliases follows.

## 2 Owner binding
A PREPARED candidate binds the full capability values: owner, image, image
generation, current execution generation, manager/zone epoch, map identity, owned
layout (globals_begin, A5, below/above sizes, stack base/top), the sole inventory
entry, expected g, the 206-byte reservation, and 1..2 bindings {ID, extent 108,
view, 1032-byte port}. During the root, comparisons use complete immutable control-field snapshots
of every binding/view and relevant ownership/control identity; padding is not
authority. The snapshot is made and validated by the actual owner, never copied
from a caller assertion. Guest record contents remain opaque mutable app data.
These are equality checks, not collision-resistant digests
and not authentication against hostile native code. Port pixels are not
snapshotted and are not a new opaque digest. There is no caller-certified seal and
no invented LOADED-payload hash.

## 3 P1 - Native inactive admission transaction
The admission runs as one native call, in this order:

1. Run the unchanged SPEC-0034 section 4 / SPEC-0035 refusal order against the
   canonical loaded world. This includes the all-zero application header. The
   world is authenticated before any profile-specific value is considered.
2. Run pure candidate validation: SPEC-0043 checks the table with current=0,
   and P3 cross-disjointness from section 5 is checked. Capture the owner-controlled
  complete binding/control snapshot under the same exclusive admission. A caller
  supplying a profile tag, claimed digest or healthy flag grants no authority.
3. Preflight with the map and pin translated destinations for (A5)..A5+3, the
   current cell g..g+3, the root token push and the initial frame.
4. Publish the single-use claim and consumed generation.
5. Install g at (A5), set current=0, push the root token and publish the initial
   frame. Setup is trusted native admission, not a guest instruction. The guest
   has not entered yet. No step after the claim may fail.

If steps 1-3 refuse, there is no guest effect, no claim and no stranded g. Only
diagnostics may change. Unsafe native refusal also preserves the report, as in
SPEC-0035. There is no modeled validation or failure branch in step 5: every destination
was already established and is held live under the exclusive lease. Native
storage lifetime/coherence violations are outside this C API's safety promise;
do not disguise a late lookup as an infallible commit.

After admission, the header check for this profile is: (A5)..A5+3 == g exactly;
the other 28 application-header bytes and the jump table keep their existing
checks. Guest writes to (A5) and to any above-A5 byte still refuse. There is no
widened dirty flag, no temporary g->0 rewrite for integrity checks, no fake or
borrowed context and no protected-image mutation outside admission or the final one-way
canonical retirement below. The exact-g predicate requires the whole owner-bound
profile/generation/epoch/map/layout/lease record to validate first; the tag alone
never selects a weaker predicate. Default/fixed mode still requires zero.

## 4 P2 - Owner environment span and commit plan
The class `owner-env-span` [E] exists only inside the profile coordinator. Each
GetPort/SetPort declares these spans:
- read (A5)..A5+3, the seal;
- read g..g+3, the current cell; SetPort also writes it. This read-modify-write
  of the same four bytes is the only allowed self-overlap;
- validate the full owned mapped108-byte extent of EVERY registered record on
  both calls, including nonselected entries. These are span/permission checks,
  not parsing, hashing or copying private record contents. Then validate current
  and Set target identities independently using the unchanged SPEC-0043 core;
- write the GetPort output d..d+3. This must be below A5, even, and must not
  alias the current cell, the reservation, any record, the stack or the header;
- the stack parameter and the 6-byte scratch, as in SPEC-0042 section 8.

The SPEC-0042 section 10 order (1)-(7), the word/frame/range rules and the failure
classes still apply. Alias checks cover both logical ranges and translated
physical bytes, using overflow-safe half-open arithmetic. Before any effect, the
coordinator preflights and stages all scratch, data, frame and native buffers and
pins translated destinations. During commit there is no fallible map lookup, no
callback, no revalidation and no allocation. Commit writes scratch SR/PC, then the
current cell (Set) or d (Get), then the frame A7=S+4, PC+2. A success counts as
one instruction. All other D/A registers, SR and the caller frame stay intact.

## 5 P4 - Lease, bindings and lifetime
The image's sole execution participant holds an exclusive lease. It covers the
capability, views, ports, all controls and outputs, and the original and current
guest backing inventory. While anything is borrowed, no new external owner is
admitted, and nothing is labelled reference-free.

Bindings: at most 2. Pairwise cross-disjointness is required among views, ports,
the capability, controls, reports, map backings and guest data, including exact
sharing. Native descriptors and views are immutable, and their storage must be
truthful.

Borrows are root-local. The view and capability borrow ends at synchronous
quiescence after a normally ended root (SPEC-0034 STOPPED conditions). After
that, the image keeps only scalars it controls: profile tag, g, generation,
epoch, consumed-generation tombstone, stop state and the frozen scalar owned-layout
reservation/record identities needed to validate cleanup. Existing image-owned
map/lifecycle references remain subject to their original contracts. Retirement
uses current owned image/map/header integrity, never stale view snapshots or
native pointer values that belonged to an ended borrow. Reports never export a
native view pointer.

If quiescence cannot be established, the image is quarantined and the borrow is
reported as unresolved, never as released. The caller then has this obligation:
keep the capability, views and ports live and unchanged while that borrow is
unresolved. No current API may release, destroy, reinitialize or recycle them on
that basis. A future recovery contract would have to prove quiescence AND release
the borrow; destruction alone is not proof. If a caller cannot meet this
conditional lifetime obligation, refuse before claim. No safe dangling-pointer
detection or recovery is promised. Quarantine never becomes a success cleanup.

## 6 Fault split
- Inert service refusal (pre-effect CPU/policy stop): malformed or unregistered
  guest IDs, unknown current ID, bad destination, bad frame or word, patched
  slot.
- Quarantine: once admitted, a mismatch in trusted generation, epoch, map,
  layout, binding, view or control snapshot, or (A5)!=g. Earlier effects and the
  actual frame are retained.
- Changes to the content of an opaque guest record are not corruption, because
  phase 1 has no private record format.
- An inactive bad candidate gets a pure validation refusal before any effect.
- SPEC-0043 itself has no quarantine API, and none is added here.

## 7 Retirement (one-way cleanup)
Retirement accepts only STOPPED-CLEAN or STOPPED-GUEST-FAULT with current owned
integrity: same generation, epoch and inventory, (A5)==g, and matching complete owner-controlled scalar
profile state, frozen layout/ranges and the actual image/lifecycle ownership.
Former native view storage need not remain live after proven root quiescence.
It does not require a valid current-cell ID, because the guest may
have stored an arbitrary ID before a service refused.
Admission additionally requires proven synchronous quiescence, zero unresolved
borrow and no quarantine, regardless of a stale STOPPED label: root/service/
callback/patch/driver depth0 and no pending exit, established by the actual owner.
If this cannot be established, no pin/scrub/header-clear/revocation/RELEASE is
authorized; preserve the unresolved borrow, stop/fault evidence and reservations.

1. With the g seal still intact, validate all scrub and header destinations and
   pin them. Check the protected 28 header bytes. Map or identity failure here
   quarantines and clears no fault.
2. Zero the declared below-A5 range canonically. This includes the reservation
   and the current cell. Stack bytes are preserved.
3. Zero the trusted A5 anchor bytes (A5)..A5+3.
4. Revoke graphics selection authority in a non-reenterable RETIRING state. Keep
   the consumed-generation tombstone, exclusive image reservation and cleanup
   phase/accounting even though the guest anchor is now zero. No ordinary entry
   or external RELEASE can exploit an intermediate canonical-looking world.
5. Only after true canonicalization, perform the owner-internal handback to the
   unchanged original SPEC-0033 RELEASE. Publish RETIRED only on its success.

The pinned below-A5/header scrub has no fallible per-byte lookup or callback.
A subsequent handback/RELEASE failure retains the consumed-generation tombstone,
actual scrub/header/handback phase counts and all unreleased reservations, and
quarantines. Do not reset ownership to EMPTY or restore g to retry.
The first-stop and
first-invariant-fault snapshots stay immutable. There is no recovery from
quarantine, no resume, no second run in a consumed generation, no hidden view, no
rollback and no automatic cleanup.

Changes needed in the old image code (not claimed present): its integrity
predicate must accept expected g only after the complete owner-bound profile,
lease,identity and phase validate; a tag alone grants nothing. Its retirement must add step 3 and the protected RETIRING/handback transaction,
including retained consumed-generation protection on failure.
Default paths keep expecting zero.

## 8 Architecture
The owner semantics are identical on native 68000 and x86. The interpreter is
only the legacy application boundary, not the PC OS architecture. There is no new
ABI, SDK, kernel or loader promotion.

## 9 Next bounded implementation proposal
Freeze the new native owner API and independent test contract before code. The
next proposal should combine real owner validation/snapshot comparison, exact-g
integrity, one-use admission, pinned environment-plan completion and one-way
retirement behind this single opt-in profile. Verify meaningful unchanged-mode
and failure-retention regressions on both native architectures. Do not spend a
separate milestone implementing a detached duplicate of SPEC-0043's validator.

Canonical guest GetPort/SetPort binding and the original selection-only workload
still require the P5 admission and P6 declared CODE0 bounds to be satisfied and
explicit owner implementation scope. No five mandatory separate approval rounds
are introduced; one accepted bounded implementation plan may include dependent
pieces. A selected-port drawing adapter/full manager remains later work.

Evidence still missing: every case in the unexecuted plan, physical-alias controls,
quiescence/lifetime and handback tests, frozen native owner API, P5/P6 implementation,
paired code/compiled size/stack measurements and human provenance review.
