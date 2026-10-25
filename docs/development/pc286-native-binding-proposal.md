<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# PC286 native binding proposal (P286-2 next gap)

Status: PROPOSED. This is not a finalized contract and not proof. It adds no C or fixtures; intake provenance is SRC-0462. Names below are illustrative placeholders. The existing numeric models (`cs_bm_*`, `cs_bc_*`) stay unchanged. The prior evidence (176 plan assertions, 32 catalog oracle cases) was not rerun, and the new tests below were not executed. This is separate from the paired release's live input lane. It has no boot, far-cast or runtime code, no callback ABI redesign and no performance claim.

## 1. Gap
`cs_bm_plan4` gives a descriptive logical recipe. `cs_bc_validate` gives numeric admission of below-1MiB catalog entries. Neither shows that a native pointer is live, that its object extent is true, that its lifetime is stable, that physical aliases are absent, or that a commit cannot fail. This proposal sets the next bounded contract between those models and a future native backend.

## 2. Numeric admission vs trusted grant
- NUMERIC admission is what C can check from labels: geometry, permissions, id coverage and physical-span disjointness. It is not a native check.
- A TRUSTED grant is a provider statement. C cannot verify pointer validity, object extent, liveness or far physical aliasing from numbers.
- The provider obligations below are axioms. They are not successful checks and not graceful malformed-guest-pointer tests. Violating these obligations is outside the contract and may cause native undefined behavior; it is not a validated guest refusal.

Provider obligations, per bank id in the bound catalog:
- P1: the grant covers at least `bytes` of backing, as native chunks of at most 32767 bytes each. The current maximum bank is 65535 bytes, which needs THREE chunks (32767+32767+1). A logical 65536-byte region needs multiple banks; no 65536-byte bank is admitted. Bank IDs remain unique in 1..64.
- P2: each chunk is live and stable from bind until unbind.
- P3: no two grants alias, including physically through far representations.
- P4: the chunk representation, canonical physical relation, alignment and far conversion are qualified by the backend.
- P5: byte stores to a chunk do not fail and do not move the backing.
- P6: no unbind or remap runs concurrently with an operation.

OPEN: representation, canonical physical relation, alignment, far conversion, ABI and link proof. Nothing here closes them.

## 3. Binding epoch
A backing-map/binding epoch is a 32-bit counter owned by the binder. It changes only when the bank-to-native chunk binding changes (bind, unbind, remap).
- It is separate from Memory Manager heap, zone and object generations.
- Guest writes and guest-handle moves do not change it. Fixed guest RAM is not remapped by them.
- No historical behavior is claimed. Callbacks are not globally frozen.
- A native pointer borrow lives for one operation only. It is released before any callback or reentry boundary under the existing contracts.
- A caller epoch value is a staleness check, not a lifetime proof. Exhaustion must refuse before a binding change; wrapping or resetting while old epoch references can survive is forbidden (D3).

## 4. Options compared
**A. One-shot checked store4.** One call preflights, stages, then commits. No native plan or pointer is retained across the call. The window of exposure is the single call.

**B. Prepared opaque native capability.** It is held across a short no-callback, no-remap critical interval. It saves repeated preflight. The cost is a new object that must be unforgeable, released on every path and invalidated on remap, which a descriptive `cs_bm_plan` cannot be. It also invites retention bugs.

**Recommendation: A for the first slice.** `cs_bm_plan` stays descriptive. It is never a token and never infallible. B is deferred until a measured preflight cost justifies it (D1).

## 5. Proposed one-shot contract (illustrative `store4`)
Inputs: `map`, bound catalog `cat` (as in `cs_bc_validate`), provider binding handle, caller epoch `e`, `at`, 32-bit `value`.
Output: a result code only. There is no pointer, plan or token output.

Ordered steps:
1. Null or invalid arguments give ARGUMENT.
2. Validate the complete map (`cs_bm_validate`) and numeric catalog (`cs_bc_validate`) for MAP/BACKING on each call in the first experiment (D4). Map/catalog arrays must be truthful, live and immutable for the whole operation; null checks cannot validate arbitrary native pointers.
3. Compare `e` with the current epoch. A mismatch gives STALE.
4. Run the `cs_bm_plan4` logic into a local plan. Odd or invalid spans give ADDRESS. Missing WRITE gives PERMISSION. This covers all four bytes. No region merging.
5. For every piece, resolve bank, offset and length to native chunks. A piece may split across chunks, so a 4-byte write may touch up to 4 chunks. Each chunk length is at most 32767. Any missing binding or chunk-length mismatch gives BINDING. Acquire every required borrow and complete every fallible lookup here, before COMMIT; retain all borrows until the last store. The binding stays exclusively stable throughout preflight and commit.
6. Stage the four big-endian value bytes into a local array before the first store.
7. COMMIT begins. All borrows are already acquired. The commit does at most four byte stores, with no allocation, IO, callback, failable lookup, metadata change or per-byte generic callback. All lookups were resolved in step 5.
8. Release the borrows without failure or callbacks, then return OK.

All refusals (steps 1-5) occur before any guest-data write, and guest memory is unchanged. After step 7 begins there is no refusal path. "Infallible" is conditional on P1-P6 and on exclusion of invalidation. It does not cover CPU or native faults and is not proof of ownership.

## 6. Effect/refusal table
| Condition | Result | Guest memory | Epoch | Output |
|---|---|---|---|---|
| Null/invalid arg | ARGUMENT | unchanged | unchanged | none |
| Bad map geometry | MAP | unchanged | unchanged | none |
| Bad catalog numerics | BACKING | unchanged | unchanged | none |
| Epoch mismatch | STALE | unchanged | unchanged | none |
| Odd address or out-of-24-bit four-byte span; no single covering region | ADDRESS | unchanged | unchanged | none |
| No WRITE | PERMISSION | unchanged | unchanged | none |
| Unbound bank or bad chunk length | BINDING | unchanged | unchanged | none |
| All checks pass | OK | four bytes written, BE | unchanged | none |
| Provider axiom violated | undefined | undefined | undefined | not a test case |

Each new code is a proposal. Existing `cs_bm_result`/`cs_bc_result` values and ordering do not change.

## 7. Backend constraints
- Native 68000/x64 keep their flat fast paths.
- Any C16 backend must later supply the actual compiler/model representation proof and measured partial costs. No zero-overhead, 128K or speed claim is made.
- There are no per-byte callbacks and no whole-region staging. Staging is four bytes only.
- No production guest-view or Toolbox handler ABI changes in this slice.

## 8. Proposed tests (NOT executed)
Executable cases use a fake provider that satisfies P1-P6.
1. Four bytes inside one chunk: OK, BE bytes, epoch unchanged.
2. Write crossing a 32767 chunk boundary inside one bank: OK, pieces split correctly, neighbors untouched.
3. 65535-byte bank bound as three chunks (32767, 32767, 1); an even-address write crossing its final chunk (and, if needed, the next bank in the same region): OK. A proposed 65536-byte bank gives MAP.
4. Write crossing two banks in one region: OK, per-bank bytes correct.
5. Write across adjacent regions: ADDRESS, memory unchanged.
6. Region without WRITE: PERMISSION, memory unchanged, even if the first bytes are writable.
7. Odd `at`: ADDRESS, unchanged.
8. `at` where `at+4` exceeds 2^24: ADDRESS, unchanged.
9. Stale epoch: STALE, unchanged; the same call with the current epoch: OK.
10. Last-bank chunk missing from binding: BINDING, no earlier bank written.
11. Chunk length sum mismatch against the bank length: BINDING, unchanged.
12. A guest write and a guest-handle move between calls leave the epoch unchanged; a bind change increments it.
13. Late invalid bank (map bad in a late slot): MAP, unchanged.

NOT executable (provider axioms, no validation test): pointer validity, real extent, liveness, physical alias freedom, stable lifetime, no faults and no concurrent unbind. Numeric chunk-length sums are executable metadata checks, not proof of those axioms.

## 9. First implementation experiment: prerequisites
- Lead decisions D1-D5 below.
- A fake host-memory provider that models chunks of at most 32767 and an epoch.
- Reuse the unchanged numeric models as the oracle for steps 2 and 4.
- A written chunk-representation statement (P4) for the target compiler, still OPEN.
- Tests above executed by the lead, with results recorded separately.
- No far casts, boot or ABI work in the experiment.

## 10. Still blocking native binding
Qualified far conversion and alignment; physical alias proof; chunk lifetime across callbacks; unreviewed modules (`guest.c`, `m68k-bus`, `m68k-session`, trap gateway); the handler span ABI; measured costs; a real provider implementation and its axioms review.

## 11. Decisions needed from the lead
- D1: choose A (recommended) or B.
- D2: confirm the new codes STALE and BINDING or fold them into existing codes.
- D3: epoch width and wrap policy, and who owns the counter.
- D4: revalidate the complete numeric map/catalog per call for the first experiment; cached validation is deferred until immutable ownership and invalidation are qualified.
- D5: whether a provider that cannot meet P5 may stage through its own transaction instead.

## Lead disposition (engineering only)
SRC-0462: adopt with changes as a PROPOSED design, not an implementation authorization. D1 chooses one-shot preflight/commit; D2 retains separate illustrative STALE/BINDING results pending API review; D3 requires no-wrap exhaustion; D4 chooses complete per-call numeric validation initially; D5 defers transaction-capable providers. Address zero is valid when its four-byte span is admitted. All borrows precede commit. The worker's 65536-byte-bank example and bind-only validation shortcut were rejected against the current contracts. Map/binding changes, unlike guest-handle relocation inside fixed RAM, invalidate the epoch. No new test above was executed. Far conversion, physical ownership, C16 link/ABI and measured costs remain open.
