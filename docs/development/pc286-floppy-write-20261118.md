<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Native286 single-sector floppy writes

2026-11-18. Canonical receipt for SRC-0620 / SPEC-0126 / IMPL-0156 /
TEST-0217. This original provider supplies one-sector persistence through the
explicitly writable BootHush built-in BIOS model. The existing read provider
is unchanged. This is not an MFS writer, multi-sector transaction, selected
System Disk modification, physical BIOS driver or System boot result.

## Implementation and source boundary

`platform/pc286/boothush-floppy-write.[ch]` binds a caller-owned range and
512-byte near bounce buffer. Each call validates state/range/bounce placement,
copies a complete live source sector, then invokes exactly one AH03/AL01
operation. It distinguishes pre-I/O argument/range/state refusal, modeled write
protection and other BIOS failure. The source and control fields are never
changed by the operation. Bounce effects are permitted after admission.
Source/provider/bounce physical aliases and invalid object lifetimes remain
caller preconditions; numeric admission is not a pointer-validity detector.

The original wrapper retains returned AH and CF before restoring native
registers/FLAGS and clearing DF. Numeric bounce placement reuses the existing
SPEC-0113 helper. No existing provider, shared storage API, filesystem or
product entry was edited. Separate successful calls commit independently;
there is no rollback claim across calls or claim of crash-safe file updates.

Source basis is the project's admitted read-provider geometry/native contract
plus independently authored BootHush BIOS behavior, SRC-0620. No third-party
CPU implementation body, firmware, owner media, private HQ or excluded
research was used. Codex authored the provider and original fixture/harness;
worker-specific usage and active-effort measurements were not available.
An independent coordinating worker read SPEC-0126 and the writer header/body;
no concrete defect was found under the declared stable small-model and modeled
firmware assumptions. That review checked range-before-narrowing, CHS/bounce
bounds, the single-sector copy/call and AH/CF/register handling. It did not
review the harness or execute tests; human provenance review remains separate.

## Exact qualification

The first complete pinned run passed at
`C:/Repos/.task-temp/pc286-floppy-write-20261118-a/report.json`, SHA-256
`cfcc0e62313c8a3ed7bd03745be82664130fd6a63b11fe8430f9f547d65245cd`.
The report owns all commands, eighteen object/import audits, twelve native
observations and exact declared input hashes. Compiler members were checked
against `scripts/pc286-toolchain-lock.json`; source, selected tools, emulator
and the supplied model contract were hashed before and after qualification.
All remained unchanged. Generated media, exported private copies, objects and
verbose logs remain in the external output directory.

| Input | SHA-256 |
| --- | --- |
| Writer C source | `43fd0f7a5e506d27acceb794a6518cb15f6ff06c803e92d3752b2e540001d9d7` |
| Writer header | `eeff136564b4df9738895e1547807fbcdb998929b36a3336e5fe3bcf02e73f4e` |
| Original native probe | `e000efcd274ab1393f624072a1307755ee5401fd5e8f282d6dcd8fdb9ff08a98` |
| Test harness | `2d0861ea99f48787b583d24ed246b48b4a3146ae6cabe53be6768689c4998589` |
| SPEC-0126 | `650cbe84ae361eead26fe63f56a10b846e362d098ec574e22fc8a4b5ae530ca1` |
| BootHush executable | `3d48d7515d3492ac796d3899ed2c47603d3330e3f0f64352fd29ed6a78284ff4` |
| BootHush original `src/pc286.cpp` contract | `d1bfe30af468878d22bbb4906ccd914b5b1967b71ac8bec452dfa6fdadb12816` |

Run `scripts/Test-PC286FloppyWrite.py` with the pinned Watcom tool root,
the admitted twin-b BootHush executable, `--model-contract` pointing to its
original `src/pc286.cpp`, and a new external `--output` directory. The harness
uses only generated original 1.2MB PC media and new `--floppy-out` destinations.
It never supplies owner media or replaces an input path.

## Results and controls

Three independent optimized boots each wrote physical sectors60,74,75,89,90
and2399 using separate provider calls, covering the declared first/final
sectors and head/cylinder boundaries. Every full exported 1,228,800-byte
image matched the expected original image with exactly those six replacements.
Each exported copy then booted a new machine without write opt-in. The native
probe recognized and verified all six persisted sectors before attempting
any write, so a second write cannot manufacture the persistence result.
The unoptimized build passed the same write/fresh-boot pair.

| Native case | Runs | Expected marker | Observed modeled cycles per run |
| --- | ---: | --- | ---: |
| Optimized writes and same-guest reads | 3 | BB | 2,197,181 |
| Fresh optimized read-only boot | 3 | BC | 838,845 |
| Unoptimized writes/readback | 1 | BB | 4,201,985 |
| Fresh unoptimized read-only boot | 1 | BC | 1,655,775 |
| Default read-only refusal | 1 | BD | 868,231 |
| Wrong-head mutation | 1 | E2 | 437,974 |
| 511-byte bounce-copy mutation | 1 | E3 | 500,316 |
| Ignored write-protection mutation | 1 | E4 | 408,142 |

The three optimized input hashes match
`9a07fe3ab20d2d705581a1bc04377d8c96b7ccf451995b070d752ecfd332bd32`;
their three persisted output hashes match
`3b9623e6ec3a5be78b376ead968779de1dc0ac9e8bd9998af9614a65fba04393`.
Every input hash remains unchanged, including fresh-boot snapshots. Each run
reports the expected POST with CS=0x1000 and the entire loaded native payload
byte-matches its build. Wrong-head and truncated-copy controls also compare
the entire exported image to their exact deliberately corrupted one-sector
result, proving the test observed the intended mutation rather than an
unrelated stop.

Native validation additionally covers complete preservation after invalid
bind, zero/oversized ranges, out-of-range and UINT32_MAX LBAs, null source,
exact bounce alias, stored-segment mismatch and malformed controls. Source
bytes, surrounding canaries and writer controls survive every call. The pure
numeric helper tests exact native segment, physical64KiB and conventional-RAM
boundaries without constructing invalid pointers. Caller IF and native DS are
preserved; DF is clear after the BIOS boundary. Generic BIOS errors other than
write protection are classified by code but are not injected by this suite.

All eighteen objects identify80286/USE16 and import only declared project
symbols; final links contain no runtime library. The optimized writer is361
text bytes, unoptimized602, with no mutable segment bytes in either object and
one helper import. Full payloads are2,060 and3,342 bytes with2,592 BSS bytes.
Cycle counts include loader, validation, pattern checks, BIOS calls and reads;
they are not isolated write latency, physical timing or a full memory budget.
Only native small-model code is admitted here. Physical persistence, FDC-port
writes, filesystem metadata updates and end-to-end System use remain separate.
