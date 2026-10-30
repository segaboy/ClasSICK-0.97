<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# PC286 checked access over bound spans (isolated experiment, slice A)

Status: adopted as an isolated experiment under SPEC-0086, layered on SPEC-0069 section 1 and SPEC-0082 spans.
Baseline: public `b68bef1`. This is one bounded slice:
- `span-bind.h/.c`: one shared environment and binding walk;
- `span-access.h/.c`: `cs_bx_read`, `cs_bx_write`.

There is no native-pointer output, no capability, no production `guest.c`
change, no link, boot or target run, and no performance claim. SPEC-0069 stays
authoritative wherever this text is silent.

## 1. Environment (`cs_bx_env`)
The environment holds `map`, `catalog`/`catalog_count`, the RAM envelope,
`spans`/`span_count` (SPEC-0082 output order), `current_epoch` (owner) and
`expected_epoch` (caller snapshot).

`cs_sb_env` refusal order:
1. **ARGUMENT**: NULL environment, catalog or spans; `span_count` 0 or above 192;
   either epoch zero.
2. **ARGUMENT / MAP / BACKING**: `cs_bc_validate`.
3. **STALE**: `current != expected`.

`cs_sb_walk` gives BINDING unless the spans follow map bank-array order, with
1..3 per bank, contiguous from offset 0, each 1..32767 bytes with non-NULL data,
covering each bank exactly and with no extras. It is metadata only and touches
no data.

## 2. Frozen refusal order
| Step | Read | Write |
|---|---|---|
| 1 | ARGUMENT: `out == NULL` or no READ bit | — |
| 2 | `cs_sb_env`: ARGUMENT, MAP, BACKING, STALE | same |
| 3 | WIDTH: width not in {1,2,4} | same |
| 4 | ADDRESS: misaligned word/long | same |
| 5 | `cs_bm_check(at, width, access)`: ARGUMENT for access bits outside 7; ADDRESS for zero/out-of-space span or no single covering region (adjacent regions never join); PERMISSION | same, access = WRITE |
| 6 | BINDING (`cs_sb_walk`, then per-byte resolution) | same |

BACKING and STALE are **deliberate insertions after MAP**, and BINDING comes
after every guest.c check. This order is not identical to guest.c whenever the
environment or binding is faulty. When the environment is valid and the binding
complete, those three codes cannot occur. The access parity test checks that,
in exactly that state, result codes, read values and resulting bytes equal the
unchanged `core/memory/guest.c` on the same logical layout. That covers 300000
fixed-seed operations; the cases are author-generated.

## 3. Effects
All `width` destinations are resolved before the first load or store. A write
stages its bytes with constant shifts (no variable shift, no 64-bit) and then
performs exactly `width` byte stores. A refusal leaves `*out` and every bound
byte unchanged. Success changes exactly `width` bytes, big-endian. No native
pointer leaves the call, and none is retained.

## 4. Caller obligations (from SPEC-0082 and P1-P6; not verified)
- Truthful live extents and physical nonaliasing of all spans.
- Exclusive, stable map/catalog/spans/backing from validation through the last
  store; no concurrent remap, callback or reentry.
- Disjoint metadata, output and backing.
- On C16, each metadata array (map, catalog, spans) fits one addressable
  segment window.
- Infallible byte stores.

## 5. C16 notes and qualification still required
Each call uses two 64-byte index arrays on the stack (stack high-water not
measured). Checks use 32-bit add, subtract and compare only, with constant
shifts. Span-pointer adds are a single 16-bit in-span delta.

Proposed separate profile: `scripts/Probe-PC286BankPlan.py --span-access`. Its
closure is bank-plan + catalog + span-bind + span-access, `mc`/`ml` x `od`/`os`.
The expected publics are `cs_bm_*`, `cs_bc_validate_`, `cs_sb_env_`,
`cs_sb_walk_`, `cs_sb_find_`, `cs_bx_read_` and `cs_bx_write_`, with no
unresolved imports. The independently run C16 closure and its two near-data refusals are recorded in [the qualification receipt](pc286-span-access-evidence.md). These compiler results do not qualify target execution.

On the pinned 16-bit compiler, `span-bind.c` refuses compilation when a data pointer is not four bytes. This guard prevents near-data compilation of the checked-access closure; it does not prove ownership, aliasing, segment lifetime, target runtime or ABI.
