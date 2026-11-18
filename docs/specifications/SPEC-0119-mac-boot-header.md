<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0119 - Bounded Macintosh startup metadata

2026-11-18; SRC-0617; IMPL-0149 / TEST-0210. This original data-only reader
acquires startup configuration from logical blocks 0-1. It neither loads nor
executes disk code and establishes no original Macintosh 128K startup ABI.

## Published facts and profile

SRC-0617 scopes the published Apple *Inside Macintosh: Files* (1992), chapter 2,
printed 2-57 through 2-59, and Apple Technical Note DV03 (March 1987). These later
sources describe the common header fields below; they do not prove January 1984
behavior. The later manual's conflicting version-$D dispatch descriptions are
not resolved or implemented. The signature acceptance rule below is this
reader's explicit profile; its 1992 source labels the signature as HFS.

All offsets are bytes from the start of logical block 0. Numeric fields are
unsigned big-endian bit patterns, not validated counts, sizes or policy.

| Offset | Bytes | Data-only meaning |
| --- | --- | --- |
| 0 | 2 | Signature; this profile accepts 4C4B hexadecimal |
| 2 | 4 | Excluded entry area; never decoded or exported as metadata |
| 6 | 2 | Opaque header version |
| 8 | 2 | Opaque page flags |
| 10, 26, 42, 58, 74, 90, 106 | 16 each | Counted names: System, shell, first debugger, second debugger, startup screen, startup application, scrap |
| 122 | 2 | File-control-block count |
| 124 | 2 | Event count |
| 126, 130, 134 | 4 each | Raw 128K, 256K and other system-heap fields |

Each name has one length byte and capacity for fifteen bytes. Names remain raw
counted bytes, including zero lengths, embedded zeroes and high bytes. No C
string, validity, collation or historical filename default is inferred. The
reader does not inspect bytes 138-1023 or use the excluded entry area.

## Contract

`cs_mac_boot_header_decode(area, length, out)` requires an exact 1024-byte input.
Null pointers, exact input/output equality, or any other length return ARGUMENT
before reading input. A wrong signature returns SIGNATURE. Any of the seven
name counts above fifteen returns NAME. Only after all validation succeeds are
the numeric fields and counted names copied into `out`; unused name tails are
zeroed. No retained pointer, dispatch target or executable bytes are returned.

`cs_mac_boot_header_read(device, scratch, out)` requires non-null distinct
control/output objects, a callback and positive device block count. Invalid
controls return ARGUMENT; a one-block device returns RANGE without I/O. The
workspace contains separate 1024-byte acquisition and staging arrays. One
SPEC-0111 request acquires exactly LBA 0 and LBA 1 before decoding the header.
Provider UNAVAILABLE, IO and unknown results map to the corresponding reader
statuses. Acquisition failure preserves the entire acquisition array as well as
`out`; staging/provider effects may remain. A valid acquisition followed by a
metadata refusal may change the workspace but preserves `out`.

Every non-OK result preserves the complete output object. Successful members
are defined; native structure padding is not serialized. Truthful live spans,
mutually disjoint controls/input/output/provider backing, stable exclusive
access and no reentry are caller preconditions. Each native object and span
must fit its native segment. Arbitrary overlap, far aliases and real pointer
validity are not dynamically detected. The fixed 2048-byte workspace and all
loop indices fit the existing 16-bit native limits. No allocation, native
structure overlays or guest-pointer conversions are used.

## Verification scope

Original tests cover endian-asymmetric scalar fields, every raw version,
all seven name lengths 0-255, all truncated input lengths, odd input alignment,
ignored entry/trailing bytes, canaries and complete failed-output preservation.
Provider fixtures exercise both read positions, unavailable/I/O/unknown statuses,
short devices and atomic acquisition. Paired original-media probes use the
current modeled Mac IWM and 286 floppy providers; their numeric-only reports
must agree. A second-sector failure must preserve both acquisition and output;
malformed names and signatures must preserve output after successful reads.
These are configuration-read results, not OS startup, legacy-code execution,
System Disk boot or physical hardware qualification.
