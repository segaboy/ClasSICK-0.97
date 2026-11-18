<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0114 - Modeled Mac128K read-only floppy provider

2026-11-18; SRC-0615, IMPL-0144, TEST-0203. Native provider contract for
SPEC-0111. The coordinator admitted the narrowly model-only own-BootHush
interface on 2026-11-18 under the 2026-11-14 owner amendment. Historical source,
physical-driver applicability and independent provenance review remain open.

## Intended scope

An original freestanding 68000 implementation reads logical 512-byte blocks
from the modeled internal, single-sided 400 KiB IWM medium. It will use actual
guest device reads, not an embedded sector array or a host-memory shortcut.
BootHush accepts raw and validated Disk Copy 4.2 images; the guest reads their
common modeled wire representation and does not parse the container itself.
Tags are covered by transfer integrity checks but are not part of the logical
block result. The selected historical System Disk is excluded from this slice.

This is a BootHush-specific diagnostic capability. It will not establish
physical seek/spin-up timing, magnetic flux behavior, IWM mode completeness,
System Disk startup, File Manager services, writable media or persistence.

## Original API and ownership decisions

- A caller-owned native context contains transfer scratch, track state,
  bounded-read controls and diagnostic error state. No allocator or C runtime
  dependency is introduced. Context, device, output and staging are disjoint
  live native objects; arbitrary aliases are not detected.
- Binding performs only numeric/object admission and publishes a device with
  800 blocks and an original synchronous callback. It does not claim a medium
  is present or attempt a read. Refusal preserves both control destinations.
- A read rejects null objects, invalid control state and LBA >= 800 before
  touching device registers. Valid LBAs are mapped across 80 tracks, with
  12, 11, 10, 9 and 8 sectors in the successive 16-track zones.
- All IWM polling, header searches and motor/seek work are bounded. There is no
  automatic retry after a malformed sector, timeout or checksum error.
  Exhaustion returns an explicit provider error and diagnostic reason; no
  unbounded wait can consume the caller's entire run silently.
- The provider verifies a matching track/side/sector/format address field,
  its check value, the matching data-sector identifier, all legal symbols,
  payload/tag integrity and the end marker before publishing the 512 bytes.
- A callback failure may change its own scratch and device state but preserves
  its destination. The existing cs_block_read layer additionally preserves
  the caller's whole output if a later block in a multi-block operation fails.
- Reads are synchronous and do not retain the callback destination pointer.
  Writes, eject, format and protected-media export are absent.

## Observed model wire and register contract

Only the current admitted BootHush profile is covered. Its original fixture
exposes a six-bit symbol alphabet, address/data marker fields and checked sector
contents. The alphabet in iwm-read.c records those required wire values, not a
third-party implementation body. The address marker is d5 aa 96; the subsequent
seven bytes are five encoded fields and de aa. The five values are track low six
bits, zero-based sector, track high bit with side zero, format 2 and their XOR.
After the data marker d5 aa ad, 706 captured bytes contain the duplicate encoded
sector number, 699 encoded payload symbols, four check symbols and de aa.

A group begins with one six-bit value whose successive two-bit fields provide
the high bits for up to three following six-bit low values. Thus three output
bytes consume four symbols. The 524-byte decoded span comprises 12 tag bytes
then 512 data bytes, so its final group produces only two bytes. Unused packed
bits for that absent third byte are ignored. All consumed symbols must belong
to the alphabet; unknown symbols fail instead of indexing outside a table.

The model's integrity relationship has three eight-bit accumulators initially
zero. At the start of each payload group rotate the third accumulator left one
bit; its outgoing high bit becomes carry. For each present byte in order, XOR
the unpacked value with the preceding accumulator (third before first), emit
that decoded byte, then add it and carry to its corresponding accumulator.
Retain the low eight bits and carry the overflow to the next byte. The three
unpacked final check bytes must equal the resulting accumulators. This describes
an admitted wire relationship, not Apple implementation or historical algorithm
provenance. Fresh C grouping/validation and arithmetic implement that relationship;
the original fixture's machine instruction sequence is neither copied nor used
as a translation template.

Register access is confined to IWM soft-switch bytes at c00000 + selector*512
and VIA port A/ddr A at effffe/efe7fe. The provider clears PA5 and makes that bit
an output while preserving other observed bits, selects the internal drive and
data mode, and enables the controller. A command selects the three CA bits then
raises/lowers the latch strobe. Model commands 4/0 choose outward/inward stepping,
1 steps and 2 starts the motor. First access issues 80 outward steps to home at
the modeled clamp, then tracks its own subsequent bounded seeks. These direction
and homing effects are checked through stopped BootHush drive observations and
actual sector results, not physical drive measurements. The provider leaves the
controller/motor enabled after reading or failure; it does not eject or write.

The capture routine uses at most 2,000,000 register polls per callback, shared
across marker search and capture. Only values with their high bit set are ready.
It returns failure on budget exhaustion. Native assembly captures a matched
address and data field before C validation, because a straightforward C reader
missed address marks at the modeled stream rate. The routine preserves the
68000 callee-saved registers and balances its stack; it does not change SR.
Exclusive drive/PA5 ownership and masked interrupts remain preconditions. This
diagnostic can block for its entire polling bound; it is not a production
interrupt-driven driver or a measured physical timeout.

`cs_mac_iwm_decode` is separately callable to verify captured original bytes.
It may modify its 524-byte scratch destination before reporting failure and is
not the block publication API. Nulls and impossible track/sector values return
ARGUMENT; address/format/end mismatches return ADDRESS; data identity/end mismatch
returns DATA; illegal six-bit symbols return SYMBOL; check mismatch returns
CHECKSUM. The callback maps all such failures and bounded polling exhaustion
to CS_BLOCK_IO_ERROR, with the particular reason in its caller-owned context.
It does not infer CS_BLOCK_IO_UNAVAILABLE from missing reads: absent media is
an observed timeout in this narrow contract. Direct invalid callback arguments
fail before hardware and leave context diagnostics unchanged.

## Required original verification

Generate deterministic original raw sectors and tagged DC42 containers outside
the repository. Build an independently written ROM probe with the project's
existing original reset and Windows-native 68000 tools. Pin all source, tools,
emulator, ROM and media hashes in a retained command/result report.

Read first/last sectors and both sides of every speed-zone transition. Exercise
sequential and backward seeks, repeated reads, invalid LBAs, no media, staged
multi-block failure and destination/canary preservation. Compare actual read
bytes with the independent fixture generator's intended payloads. A tagged
container must produce the same logical bytes while validating nonzero tags.

Use the documented persistent checksum-fault option on the second block of a
two-block operation: first staging succeeds, second fails, caller output stays
unchanged. Keep the faulty result; a separate fresh run with injection removed
must succeed. Distinguish altered returned nibbles from observed guest errors.
Add meaningful defects for checksum bypass and incorrect LBA mapping. Tests
must reject these defects instead of merely mirroring the provider's internals.
No observer counter, mounted image or successful sector read is a boot claim.

## Permitted inputs and provenance limits

Reviewed for this draft: current ClasSICK POLICY and 2026-11-14 amendment,
SPEC-0111 and original block/reset/build code; BootHush
5d613614dd4f17d605de75ecc3f38c9129079488 documentation at docs/mac-floppy.md and
its original tests/test_mac_floppy.py, SHA-256
109f00d96ef63575738d3255dbeeda0dc9362c1b756cb9811e074ada49f7007b.
No Apple,
Microsoft, MAME or other third-party implementation was inspected for this
draft. The BootHush document explicitly attributes its IWM format implementation
to MAME; that body was not read or copied. The test calls its ROM original but
does not separately document the historical origin of the exact decoder
relationship. That limitation was raised before implementation; the coordinator
admitted its wire facts only as current own-BootHush behavior, with independently
authored implementation and synthetic tests. This is not a historical-source
review or a finding about the test's originality. The model remains dependent
on its disclosed MAME-adapted BSD component. That component was neither consumed
as an implementation input nor imported into ClasSICK. A confidential manual
identified by a separate source search was excluded entirely; no contents from
it were provided to or used by this implementer.

The source, capture routine, fixture and script were authored with Codex
assistance. Synthetic tests are implementation-aware, not an independently
staffed oracle or a human provenance review. No private HQ, selected reference
disk, excluded research original or held UART body was accessed in this slice.

The expected selected test executable is the separately pinned active-profile
build with SHA-256 7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a.
Any later experiment must verify that identity and retain its model-only limits.
