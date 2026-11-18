<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Selected disk structural observation — 2026-11-18

SRC-0612/0614; TEST-0205. The exact local disk selected by the newer prepared
survey workflow is now identified and exercised read-only. The canonical
numeric receipt is [selected-disk-survey-20261118.json](selected-disk-survey-20261118.json).
The private location, transfer image, commands and full reports remain outside Git
at C:/Repos/.task-temp/system-disk-20261118-a/.

The 419284-byte Disk Copy 4.2 container has SHA-256
1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647,
409600 logical data bytes and 9600 tag bytes. Its filename label is not proof
of System/Finder version or January1984 identity. No executable bytes, resource
assets or reference frames are included in this receipt.

## Executed workload

The existing original survey files prepared in the workspace by Claude were
reviewed before execution: scripts/Survey-SelectedDisk.py and
probes/pc286/selected-disk-survey.c. Their exact hashes are in the receipt.
They use the existing original SPEC-0111/0112/0113 block/metadata/286 transport
implementation and pinned native Windows tools, not a historical implementation.

Three bounded Mac runs mount the original DC42 read-only with the released
ClasSICK ROM. Three native286 runs load an original survey payload and observe
the selected logical blocks through an explicit private PC floppy transfer.
The transfer uses sectors60..859; it is not native Mac GCR media, carries no tags
and is not claimed as the final 286 product bridge.

All Mac runs match cycles, PC and frame. The drive is present/protected but
the released ROM performs zero IWM reads or steps: it has no disk startup.
All286 survey runs match their report bytes and modeled cycle count1334641.
The entire2999-byte loaded native payload matches the constructed binary;
linked BSS is6642bytes. These are diagnostic sizes, not whole-system budgets.
BootHush executable SHA-256 is
7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a.
The original selected disk remains unchanged after every run and at completion.

The fault control fails the header read with the expected IO result and exactly
one injected firmware failure. An original no-volume fixture returns SIGNATURE.
These controls test the observation path, not System/Finder compatibility.

## Structural findings and their limits

The guest observes10 active directory records across12blocks, with no entry-bound
error, no nonzero remainder after the proposed end-of-block marker, and no read
error. The header describes391 allocation units of1024bytes, starting at logical
block16, and109 free units.

Two explicitly declared12-bit map candidates are evaluated without assuming the
winner. High-nibble-first packing yields20 consistent fork descriptions,
282used/109free/0invalid entries. Low-nibble-first yields10 bad fork links and
118invalid entries. The high-first packing therefore fits this exact image's
metadata and independently documented length/range constraints; this is an
inference from a reproducible structural observation, not execution of an
Apple filesystem or a claim about all historical variants.

SPEC-0115 adopts a strict read-only profile with that packing and consecutive
active entries/zero tails. Nonzero inactive records and ambiguous tails refuse;
it does not guess deletion behavior, case-folding, repair semantics or ownership
across files. Further original malformed-media tests are required independently.

No boot block, System/Finder code, protected asset or file payload was inspected
for implementation. The original diagnostic reads metadata and calculates chain
consistency. The file remains a private black-box runtime input; no third-party
code or disk bytes enter public fixtures. Independent human provenance review
and physical/historical/complete-OS gates remain open.

This completes exact-input identification and a repeatable storage baseline.
It does not complete System Disk startup, usable applications, writes/persistence
or either physical target.
