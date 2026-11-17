<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0113 - Modeled native 286 floppy block provider

2026-11-17; SRC-0610/0612, IMPL-0143, TEST-0202. Original transport contract
for SPEC-0111, using only BootHush's documented built-in firmware interface.
This is a native 16-bit real-mode diagnostic, not a physical IBM BIOS driver,
Macintosh GCR reader, filesystem, System boot or protected-media admission.

## Scope and binding

`cs_pc286_floppy_bind` accepts a caller-owned provider, a first physical sector,
a positive logical block count and a distinct `cs_block_device` destination.
The sole admitted medium is original raw 1,228,800-byte PC floppy media on
modeled drive zero: 80 cylinders, two heads, fifteen 512-byte sectors per track.
The logical range must fit entirely within its 2,400 physical sectors. Sector
zero is addressable; callers choose a declared subrange that does not overlap
their diagnostic bootstrap/payload when constructing fixtures.

The provider is specific to Open Watcom's 286 small model: near code/data
pointers, native DS-relative objects, one stable conventional-memory segment.
No far pointer, guest address, allocator, runtime library or BIOS replacement
is introduced. The provider owns a 512-byte native bounce buffer inside its
caller-owned context. Context, device, staging and output are live, stable,
disjoint native objects; arbitrary overlap and physical aliases are caller
preconditions, as in SPEC-0111. Exact callback destination/bounce equality is
refused. The bind function validates nulls, geometry and bounce-buffer placement
before publishing either the context controls or device. Refusal preserves
both destinations. Bind does not read the medium or discover its existence.

Both bind and read require the bounce buffer's near offset to be at most
`fe00h`; its physical address `DS * 16 + offset` must be at most `9fe00h`,
and its low sixteen physical address bits must be at most `fe00h`. Thus every
one-sector transfer stays within the native segment, modeled conventional RAM,
and one physical 64 KiB boundary. The numeric checks do not prove native object
lifetime, ownership or adequate stack space. Stable DS and live near objects
are prerequisites. Arbitrarily rebinding DS can invalidate the context pointer
itself and is outside the contract; the stored segment guard does not repair it.

## Reads and result mapping

The callback rejects a null context/destination, stored-segment mismatch under
the live-object preconditions, invalid subrange or unsafe bounce location with
`CS_BLOCK_IO_ERROR` before firmware.
A logical LBA outside the declared block count is rejected in the same way.
After admission, add it to the first physical sector; the sum is at most 2399.
For physical sector `p`, use cylinder `p / 30`, head `(p / 15) % 2` and
sector `(p % 15) + 1`. Call `INT 13h` with `AH=02h`, `AL=01h`, cylinder in
CH, sector in CL, head in DH, drive zero in DL and native bounce address in
ES:BX. One-sector operations cannot span a track. There is no retry/reset,
write, format, geometry query or interpretation of an undocumented AH status.

Carry set maps to `CS_BLOCK_IO_ERROR`; carry clear maps to
`CS_BLOCK_IO_OK` only after copying all 512 bounce bytes into the supplied
staging block. On firmware failure the bounce buffer may change, but the
callback destination does not. `CS_BLOCK_IO_UNAVAILABLE` is not inferred from
firmware status: a missing modeled medium is an I/O failure in this contract.
The callback is synchronous and does not retain the destination pointer.
The shared `cs_block_read` wrapper retains its existing whole-output
publication rule, including when a later sector fails after earlier staging.

## Assembler boundary

The original inline wrapper passes CX, DX and BX explicitly, sets AX to 0201h
and ES to the native DS, then invokes INT 13h. It saves/restores DS, ES, SI, DI,
BP and caller FLAGS on its own stack, converts returned carry to a native
0/ffffh result before restoration, and clears DF before returning to C.
AX/BX/CX/DX and condition codes are declared clobbered; CS, SS and a balanced
stack remain requirements of the admitted modeled firmware, not general BIOS
claims. Preserving FLAGS restores the caller's IF; DF is deliberately zero
under the C calling contract. No protected or third-party firmware body is an
implementation input.

## Original diagnostic plan

Generate patterned original sectors after the diagnostic's loader/payload and
at the final sector. Native tests compare actual loaded bytes at first/last,
head and cylinder transitions, and exercise declared subrange boundaries,
invalid binding controls, oversized LBAs and numeric native-buffer placement
boundaries through a pure arithmetic helper, without fabricating pointers.
Canaries and unused tails must survive. Firmware reads are real modeled
INT13 transfers, not a RAM callback or media embedded in the payload.

The documented `--fail-floppy-read C:H:S` fault can fail a chosen otherwise
valid read. Select the second sector of a two-sector provider operation beyond
the bootstrap/payload. The first sector must reach staging, the second firmware
read must fail exactly once, and all caller output must remain unchanged.
This is a deliberate firmware-model fault; it does not model partial physical
failure. A mapping defect and an ignored-carry defect are separate original
negative controls. Generated floppy and all input source hashes must remain
unchanged. The source, linked payload, tool identities, executed bytes, POST
results, faults and limitations belong in the test report. No trial is claimed
until the lead reviews this contract and the run actually completes.

An optional original SPEC-0112 integration places generated volume metadata at
logical block 2 and one known directory entry at block 4, even offset 20, within
a 128-block subrange beginning at physical sector 60. The native probe reads
the header through `cs_mfs_header_read`, reads that known directory block through
`cs_block_read`, and decodes the single entry. It checks every published raw
field, counted names including zero/high bytes, zeroed name tails and canaries.
A separate fault on physical sector 62 must return `CS_MFS_IO` with the entire
volume output unchanged. These are separate payloads and observations from the
provider-only test, not directory enumeration, map/fork validation or a System
Disk trial. The directory occupies relative blocks 4-5, outside allocation
storage beginning at block 6, and the known entry declares absent zero-length
forks. Fixtures are generated from the admitted format contract only; this does
not create or validate a complete filesystem or its allocation-map packing.
