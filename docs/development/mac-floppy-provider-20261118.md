<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Native Mac floppy block provider — 2026-11-18

SRC-0615 / SPEC-0114 / IMPL-0144 / TEST-0203. The original native68000 provider
connects SPEC-0111 to actual modeled IWM reads. It homes/seeks the internal drive,
captures bounded encoded sectors, verifies address/data identity, symbols,
checksums and end markers, then publishes512 logical bytes. Tags participate in
integrity checking. No host-memory sector shortcut, disk write or Apple ROM is used.

The final report is C:/Repos/.task-temp/mac-floppy-provider-20261118-e/report.json.
Eight scenarios pass on BootHush executable SHA-256
7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a.
Raw and original taggedDC42 fixtures each make16successful reads, spanning every
zone boundary, the final sector, backward/repeated reads and LBA191→192 in one
two-block request. Exact payload/tag, canary and unused-output checks pass.
Malformed address, symbol, duplicate-sector, check and end fields are rejected.

A second-block persistent checksum fault leaves the entire caller output
unchanged after the first block has reached staging. No-media polling terminates
with zero successful reads. Deliberately broken LBA mapping and checksum bypass
are rejected. A fresh healthy run after the fault matches baseline state exactly.
Fault nibble counters count modified observations, not failed guest callbacks.

The normal complete diagnostic ROM payload is3152bytes; tagged variant3164.
Probe BSS is0xB00..0x1814; all target links have no unresolved/runtime imports.
Normal raw/tagged16-read runs take39745344/39682642 modeled cycles.
No-media exhausts the bounded polling budget after76366302modeled cycles.
These are model observations and diagnostic budgets, not physical throughput or
complete-ROM/whole128K acceptance.

An initial C scanner missed address marks and correctly timed out. The original
assembly capture uses a tight2000000-poll total bound with an explicit native ABI;
decoding/checksum verification remains independently authored C. The earlier
failed experiment is retained. A later harness correction removed an invalid
equivalence between changed checksum nibbles and failed reads. Independent review
then required source/spec/helper identities before the first build and equality
after all runs; the final-e rerun includes that correction.

Reproduce with scripts/Test-MacFloppyRead.py, the already pinned Windows m68k
toolchain, the exact BootHush executable and a fresh external output directory.
The report contains exact arguments and source/tool/artifact hashes.

The provider is exclusive, read-only and synchronous, requires masked interrupts,
and leaves the modeled controller/motor enabled. It accepts the current format2
model. Physical delays, PWM/flux, IWM mode completeness, disk replacement, writable
media, System/Finder startup and physical validation remain open. See SPEC-0114
for ownership, error and fixed-buffer contracts. No historical-disk bytes were
used in these original fixtures.
