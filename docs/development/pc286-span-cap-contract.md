<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# PC286 checked capability and native borrow bridge (isolated model)

Status: adopted as an isolated model under SPEC-0087 and SPEC-0069 section 5, layered on SPEC-0082 spans and the
slice-A `span-bind` walk. Baseline: public `b68bef1`. Reviewed separately after slice A. There is no production binding, link, boot or target
run. Numeric conformance does not establish physical ownership.

r3 resolves the lead's slice-B findings; r4 adds the R3-2 single change rule
(section 1) and the R3-1 pointer wording (`span-bind.h`):
- **R2-1:** the registry is fresh-only and capabilities record registry identity.
- **R2-2:** each owner lifetime holds one authoritative binding, which is the
  identity domain for epochs; rebind advances before it changes anything.
- **R2-3:** the backend-helper preconditions are documented in `span-bind.h`.

## 1. Contexts
All context objects are explicit caller objects; there are no globals.
- **Registry** `{issued, current, limit}`.
- **Owner** `{map, registry, binding, lifetime, generation, generation_limit}`.
- **Capability** `{owner, registry, lifetime, generation, access, cursor, released}`.

**`cs_bq_registry_init(r, limit)`**
- ARGUMENT on NULL.
- **STATE** if `issued != 0` or `current != 0`, and the registry is unchanged.
  The storage must be zero-filled (static or explicitly zeroed) and **never
  re-zeroed or reused while any owner or capability made from it can still be
  used**. Re-initialization would recycle lifetime numbers (R2-1).
- Otherwise it sets only `limit`.

**`cs_bq_owner_open(r, binding, generation_limit, out)`**
1. ARGUMENT: NULL registry, binding or output.
2. Binding check:
   - `cs_sb_env` gives ARGUMENT, MAP, BACKING or STALE (`current_epoch` differs
     from `expected_epoch`);
   - then `cs_sb_walk` gives BINDING.
3. EXHAUSTED: `issued >= limit`.

Every refusal leaves the registry and the output unchanged. On success:
- `issued` and `current` advance, so every older lifetime of this registry
  becomes STALE;
- the owner records `map = binding->map` and **the binding itself**.

**`cs_bq_owner_advance(o)`**
- ARGUMENT, then EXHAUSTED (`generation >= limit`, unchanged).
- It **only invalidates** current capabilities. It never licenses a change to
  binding metadata or placement (review R3-2); those changes use a fresh,
  stable binding and `cs_bq_owner_rebind`.

**`cs_bq_owner_rebind(o, binding)`** (R2-2)
1. ARGUMENT.
2. **MAP** if `binding->map != owner.map`. Another map needs a new owner
   lifetime.
3. Binding check (as above).
4. EXHAUSTED.

On success the generation advances **before** the binding pointer is replaced,
so every capability opened under the previous binding is STALE. Every refusal
leaves the owner unchanged.

**Epoch domain (R2-2).** `current_epoch` and `expected_epoch` are compared only
within one binding object. Two bindings may carry the same map pointer and the
same epoch numbers; a capability can only reach the binding its owner holds now.
The test suite's cross-binding case covers exactly this: two placements of one
map, both at epoch 4.

## 2. Open
`cs_bq_open(owner, at, bytes, access, cap)` refusal order:
1. ARGUMENT: NULL owner, cap, owner registry or owner binding.
2. `cs_bm_check`: ARGUMENT, MAP, ADDRESS, PERMISSION. This is the full-span
   preflight.
3. STALE: `registry.current != owner.lifetime`.

A refusal leaves `cap` unchanged. The capability stores:
- the owner and **the owner's registry pointer**;
- lifetime and generation;
- rights;
- a cursor (bank index, bank offset, remaining bytes).

There is no epoch parameter. The epoch belongs to the owner's binding.

## 3. Liveness
Every chunk or borrow on a capability is STALE when any of these hold:
- `owner.registry != cap.registry`, i.e. the owner object was re-opened in
  another registry. This applies even when the lifetime and generation numbers
  coincide (tested at lifetime 1, generation 0 in two fresh registries);
- `registry.current != cap.lifetime`;
- `owner.lifetime != cap.lifetime`;
- `owner.generation != cap.generation`.

## 4. Descriptor iteration
`cs_bq_chunk(cap, limit, out, emitted)` refusal order:
1. ARGUMENT: NULLs, or `limit` outside 1..32767.
2. CAPABILITY: released.
3. STALE (section 3).

At the end it gives OK with `emitted = 0`. Otherwise it emits exactly
`min(remaining, limit, bank remaining)` and advances. A descriptor never
crosses a bank.

## 5. Native borrow
`cs_bq_borrow(cap, need, limit, run, emitted)`. It takes no environment
argument; the binding comes from the owner. Refusal order:
1. **ARGUMENT**: NULL run or emitted; `need` not 1 (read) or 3 (read+write);
   NULL cap, owner or registry; `limit` outside 1..32767.
2. **CAPABILITY**: released.
3. **STALE**: section 3.
4. **PERMISSION**: `need` not a subset of the stored rights.
5. **STALE**: the owner holds no binding, or `binding.map != owner.map`. This
   is an owner-invariant breach, reachable only if the caller edits the binding.
6. Binding revalidation on every borrow: `cs_sb_env` (ARGUMENT, MAP, BACKING,
   STALE), then `cs_sb_walk` (BINDING).
7. End: OK with `emitted = 0`. This check comes after the binding checks.

On success, `run = {read, write, bank, offset, length}`:
- `length = min(remaining, limit, bank remaining, bytes left in the native span)`.
- `write` is NULL unless `need == 3`.
- The cursor advances by `length`.
- A refusal leaves the run, `emitted` and the cursor unchanged.

**Borrow scope.** A run is valid only until:
- the next call on the same capability;
- any advance, rebind or new owner lifetime;
- any change to the binding, spans or epoch;
- release;
- any callout or reentry.

It must never be retained.

**Caller duties.** The caller keeps the registry, owner, binding, map, catalog
and spans alive, unmoved and unmodified while any capability made from them can
still be used. A binding that changes must be swapped in through
`cs_bq_owner_rebind`, never edited in place. Revalidation catches some breaches
of this duty (tested: NULL span data; binding map re-pointed); it is not a
substitute for the duty.

The SPEC-0082 and P1â€“P6 obligations apply unchanged (truthful extents, physical
nonaliasing, disjoint inputs/outputs, C16 metadata windows, infallible stores).

## 6. Rights and release
`cs_bq_rights` returns the stored access (ARGUMENT on NULLs). `cs_bq_release`
marks the capability released. After release, chunk and borrow return
CAPABILITY.

## 7. Evidence and qualification (author-executed, not project verification)
- **Capability suite:**
  - SPEC span lengths and 20000 random drains (48780 descriptors checked);
  - fresh-only registry;
  - cross-registry and equal-number liveness;
  - owner-open refusals with no registry effect;
  - exhaustion without wrap;
  - the cross-binding same-map/same-epoch case: rebind with ARGUMENT, MAP,
    BINDING, EXHAUSTED and OK; memory written only through the current binding;
  - the refusal ladder with unchanged outputs;
  - two contract-breach detections.
- **Window parity:** 8000 windows against unchanged production
  `cs_guest_window`, with identical codes. 1889 windows borrowed, 26502849
  bytes compared, 13884448 written through runs and observed by `cs_guest_read`.
- **Profiles:** gcc, clang, gcc ASan/UBSan, gcc `-O2`, gcc `-m32`,
  gcc `-m32` ASan/UBSan. All 7 pc286 span suites pass in each (42 of 42).
- **Defect controls (r4, strict driver):** 19 compiled mutants, each caught at
  its named intended assertion after a passing baseline (10 for the r3 rules,
  9 structural). One further mutant, the bank cap on borrow, is
  **equivalent**: a validated binding's spans are bank-local, so the span cap
  implies it. It is not counted.
- **Compiler check:** profile `--span-cap`. Its publics now include
  `cs_bq_owner_rebind_`. Pinned WCC **not run** here.
- **Stack:** two 64-byte index arrays per borrow, owner open and rebind;
  high-water not measured.
- **Still open:** link, ABI, alignment and measured cost.
