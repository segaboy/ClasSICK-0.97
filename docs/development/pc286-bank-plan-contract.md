<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# PC286 bank plan contract

Status: unbound logical-metadata and proposed-write-plan prototype. It is not far access, backing admission, commit execution or a production ABI. `core/memory/guest.c` and all runtime paths are unchanged. Files: `probes/pc286/bank-plan.h`, `probes/pc286/bank-plan.c`, `tests/pc286-bank-plan.c` (proposed, not executed by the worker).

## Geometry (SPEC-0069)
- 1..8 regions and 1..64 banks. The map, region array and bank array must be non-null.
- Region base and bytes are even, bytes > 0, and the region fits the 24-bit space.
- Flags use only values 1 (READ), 2 (WRITE) and 4 (EXECUTE), and READ is mandatory. The mask is the full 32-bit `0xFFFFFFF8UL`.
- Regions do not overlap.
- Bank ids are unique in 1..64 and bank sizes are 1..65535.
- Each region owns one non-empty, in-range slice of the bank array. Slices do not overlap and together cover every bank exactly once.
- Slice sizes sum exactly to the region bytes.
- The whole geometry is validated even if an operation touches only an early bank.

This proves nothing about physical ownership, aliasing, backing existence or commit.

## Functions
All return `cs_bm_result`: `CS_BM_OK`, `CS_BM_ARGUMENT`, `CS_BM_MAP`, `CS_BM_ADDRESS` or `CS_BM_PERMISSION`.

`cs_bm_validate` returns ARGUMENT for a null map and MAP for any invalid array, count or geometry.

`cs_bm_check` applies these steps in order:
1. access 0 or bits outside 7: ARGUMENT.
2. Validate the map.
3. Zero bytes, `at >= 2^24`, a span past `2^24`, or no single covering region: ADDRESS. Adjacent regions never merge.
4. Missing permission: PERMISSION.

`cs_bm_plan4` applies these steps in order:
1. Null `out`: ARGUMENT.
2. Validate the map.
3. Odd `at`, or an invalid 4-byte span: ADDRESS.
4. Missing WRITE: PERMISSION.

The plan lists 1..4 ordered pieces of (bank id, bank offset, length <= 4). Each piece is contiguous within one bank. A long may cross banks only inside one region. The value is encoded most-significant byte first. All unused plan fields and data bytes are zero. Any refusal leaves `out` byte-for-byte unchanged.

## Implementation constraints
- C99. The code includes only the owned `c16.h`, and the plan size is asserted <= 80 bytes.
- The code uses full 32-bit masks, constant 24/16/8/0 shifts and bounded loops with unsigned indices.
- It has no division, modulo, variable shifts, aggregate assignment, memcpy, memset, allocation, IO or far casts.
- The plan is staged in a local object and copied field by field only on success.

Staging metadata does not make a fallible native bank commit atomic. A real backend still needs an infallible, stable commit or its own transaction staging.

## Caller obligations
The map and descriptor arrays are truthful, live, aligned and immutable during the call. Writable `out` is live, aligned and disjoint from those inputs. There is no defense against hostile pointers or forged plans. Descriptor counts truthfully bound the actual arrays; a non-null pointer does not prove capacity.

A plan expires on any map change. It is not a capability, a commit token, or a generation or lifetime mechanism.

## Tests
The worker proposed these tests; lead execution is recorded in [the evidence](pc286-bank-plan-evidence.md). They cover:
- cross-bank big-endian and contiguous plans;
- late whole-map invalidity;
- no partial output on refusal;
- high flag bits;
- the 24-bit boundary;
- odd addresses and adjacent-region refusal;
- permissions;
- duplicate bank ids, slice overlap and holes;
- bank sums too large or too small;
- null arguments.
