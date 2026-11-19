<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Loading a Shell segment from disk into RAM

Both native targets used row zero of the supplied System Disk's Shell jump
table to locate and copy a **3,140-byte segment body** into guest RAM. Its
resolved entry lies **254 bytes from the start of the copied body**. Two cold
BootHush runs per target returned identical metadata. The coordinator's
copy-only builds omit the call into the loaded segment and reported zero
executions. The private DC42 input retained its pinned SHA-256, recorded in
the [preceding metadata receipt](code-stream-20261119.md).

Two generated MFS disks checked the next step. Both contain an original
620-byte segment with the selected entry at body offset 2. One routine returns
37, the other 82. The identical native 68000 ROM executed each from RAM and
returned from the call. The identical native 286 payload executed each
disk-supplied MOVEQ/RTS pair through its existing 68k interpreter, checking the
expected return PC and restored stack pointer. The result changed with the
disk on each machine.

| Probe input | Representative behavior | Full run report SHA-256 |
|---|---|---|
| Generated, result 37 | Mac/286 baseline and fresh recovery returned 37; injected late metadata read refused | `0a27d7725841945a8c4ad7526a2ea0668242224fb9e3218a7b4bf43ebfcd1f5f` |
| Generated, result 82 | Mac/286 baseline and fresh recovery returned 82; injected late metadata read refused | `41602a078ceb65e783e6157e22ead9b12243791f28ccb21da90e9bbf1619b0e2` |
| Selected System Disk, copy only | Mac/286 each repeated the 3,140-byte copy and resolved offset 254 | `0d01d72a5c297a012276e85d28fee4000ac169f117e69def02373878d80eecd3` |

The generated disk hashes were
`a963a0561e44243eb3826125fb76b16f4947c04f85c365615fd612e19ea72117`
and `92e1fda097e3297e69cfb4210742c0d7f4c0707d26b2a2af357e5fe70c389090`,
respectively. The 64 KiB ROM hash was
`7f26c28d913d4141827d9dfc9b5844a049c61249dc3dc1f1a43a20975667c49f`
(13,372 linked payload bytes, 30,556 bytes of data and BSS). The 286 payload
hash was `e57674a7ef714de395c639a2ed2dec2340d61def4ca82471072ef6bdb1e638fc`
(17,460 payload bytes, 31,790 bytes of BSS). BootHush compared all 286
payload bytes over two deterministic cold-run inspection windows and checked
the numeric result agreed across both observations.

## Interface and reproduction

Use `cs_code_stream_load` after retaining the SPEC-0127 manifest and its
resource provider. It resolves a table slot with SPEC-0132, copies its segment
body through the provider in chunks of at most 256 bytes, then publishes the
size and entry offset. The native probe reuses its completed inspection
workspace for the 16 KiB body buffer. Hosted tests exercised failure at row
resolution and each of the three subsequent copy reads. A failed copy cleared
the attempted body, retained guard bytes and preserved the output descriptor.
Optimized and ASan/UBSan configurations each passed 29 loader checks and
1,022 existing inspection checks.

Use the native toolchain, compiler and BootHush arguments from the preceding
receipt with `scripts/Test-CodeStream.py --routine-base after-header
--load-segment --fixture-result 37`, repeating with 82 and a fresh output
directory. An explicit coordinator-owned private input compiles the copy-only
path. Run reports retain exact commands, checked source identities, native
layout and tool hashes.

This is a standalone probe with modeled disk providers. Launching the disk's
Shell still requires an A5 world, callable jump-table entries and whichever
system services it invokes. The observations do not establish an installed
286 Shell launch, ROM parity, or a physical hardware result.
