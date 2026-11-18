<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0117 - Native 286 bank-backed scalar guest memory

2026-11-18. Original engineering extension of SPEC-0069 and the independently
authored bank/catalog/span experiments; IMPL-0147 / TEST-0207. This isolates an
actual native memory provider without changing the existing flat guest API,
interpreter, resource reader or manager callers. No historical OS claim follows.

## Platform and ownership

The admitted compiler is the project's pinned Open Watcom 16-bit 286 small
model: 16-bit int/size_t, near code/data and explicit four-byte far data
pointers. The caller supplies a zeroed, live, aligned service object and stable,
disjoint near input/output objects, each within one native segment. DS must
equal SS and remain stable throughout use. The service remembers its own near
identity and CS/DS/SS selectors; copying an active service does not create a
second binding. No guest address is cast to a native pointer.

The owner reserves an arena in conventional RAM, at or above physical 0x1000
and wholly below 0xa0000. Banks must fit that arena, be physically disjoint,
and avoid the complete current 64 KiB code and data/stack segment envelopes.
This excludes IVT/BDA, native code, controls, outputs and stack in the admitted
small model. It does not discover arbitrary external owners or hardware DMA.
The caller must own every admitted RAM byte, exclude MMIO, preserve mappings
and lifetime, and prevent concurrent mutation/remapping during each operation.
These are explicit trusted native-owner preconditions, not a security boundary.

## Binding and lifetime

Binding copies up to eight logical regions and 64 banks into the service.
Regions use SPEC-0069's 24-bit logical addresses, even base and positive even length,
nonoverlap, READ required and flags restricted to READ/WRITE/EXECUTE (1/2/4).
Each references a positive contiguous bank-array slice; every bank is used
exactly once, and its sizes sum exactly to that region's length. Bank IDs are
unique 1..64; lengths are 1..65535 bytes. The bank's explicit segment:offset
must satisfy length <= 65536-offset. Physical start is segment*16+offset;
physical range checks precede any pointer construction or dereference.

Bank metadata is copied, not a retained borrow. No native pointer is returned.
Only numeric admission occurs at bind; the arena is not read or initialized.
First binding gives generation 1. Only an inactive service can be rebound;
each successful binding increments its prior generation without wrapping.
The identity is the pair (this service object, generation), not a transferable
global token. Retirement requires the active matching generation, marks it
inactive and preserves the counter. Rebinding a retired object invalidates all
earlier generations. Counter exhaustion refuses and preserves the service.
Caller code must not erase/reinitialize a live object's lifetime history.

Refusal preserves the entire service and output generation. A successfully
bound object owns its copied descriptor state; callers do not edit it during
ordinary use. Defensive validation rejects malformed state, but cannot detect
all valid-looking unauthorized edits by trusted native code.

## Scalar transactions

Read takes service, expected generation, logical address, width 1/2/4, access
mask containing READ, and a native uint32 result. Write takes service,
generation, address, width and uint32 value. Both validate the complete binding
and all byte destinations before the first far-memory access. Word/long
addresses must be even; the complete operation must fit one logical region.
Adjacent regions never combine. Within that region bytes may cross any number
of bank boundaries, including four one-byte banks. Access masks are full-width
32-bit checks, with EXECUTE requiring a READ operation and matching permission.

Every resolved byte uses its bank's original segment and checked offset plus
in-bank displacement; arithmetic never carries a far offset into another bank.
The only physical pointer conversion uses the admitted compiler representation
(segment in the high word, offset in the low word). All byte accesses are
volatile. Read combines bytes big-endian into a temporary value and publishes
only after every load. Write stages the low width bytes big-endian and performs
exactly those byte stores after all validation succeeds. There is no allocator,
callback, firmware call, exported pointer or fallible step during commit.

Failure preserves read output and every backing byte. This all-or-nothing rule
depends on stable, exclusively owned ordinary RAM with infallible byte accesses;
it does not roll back hardware faults or concurrent writers. Reads/writes do not
change the binding generation. Mapping changes require retirement and rebind.

Error order: null service or malformed mandatory read output/access is ARGUMENT; then native
service/identity/selector state is STATE, inactive or mismatched generation is
STALE, malformed logical coverage is MAP and invalid physical backing BACKING.
Next come WIDTH, even-address/span ADDRESS and region PERMISSION. Binding uses
ARGUMENT for nulls/count envelope, STATE for malformed/active lifetime, MAP for
logical geometry and BACKING for native geometry, then EXHAUSTED. No unsupported
read/write is silently narrowed. These new results are not existing guest enums.

## Verification

An independently authored native 286 diagnostic reserves real RAM outside its
payload/data/stack. It checks actual far bytes and canaries, aligned word/long
operations crossing odd-sized bank boundaries, four-bank transfers, accesses
near offset ffff, a 128 KiB logical span, full-width masks, permission/bounds
refusals, stale/copy/retire/rebind behavior and invalid later-bank atomic refusal.
Meaningful implementation defects must be rejected, including altered byte
order, skipped generation checks and premature writes before failed admission.
The harness pins source/spec/helper/tool/emulator identities before building,
verifies they remain unchanged, checks no linked runtime libraries, validates
the original payload loaded in guest RAM and preserves input media hashes.

This is an actual native provider primitive, not production interpreter or
whole-OS integration. Original-fixture BootHush results are distinct from
physical 286 behavior. Existing native Mac/x64 fast paths and flat APIs stay
unchanged; a separate reviewed adapter must replace retained flat views in
segmented consumers before those consumers can claim cross-bank support.
