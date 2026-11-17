<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0143 - Modeled native 286 floppy provider

2026-11-17; SRC-0610/0612; SPEC-0111/0113; TEST-0202.
Original `platform/pc286/boothush-floppy-read.[ch]` connects a declared logical
subrange of an original PC floppy to the shared block service, using the
documented BootHush firmware interface, a checked native bounce buffer and one
sector per firmware call. Numeric near-segment, physical-boundary and
conventional-RAM admission precede transfers. Read errors are not retried.

Original Watcom inline assembly captures carry and preserves the declared
calling boundary. No BIOS/emulator implementation, protected disk, third-party
bootloader or runtime body is an input. A bounded Codex worker authored the
provider/probes/verifier; the lead reviewed the contract and source; a separate
worker reviewed native arithmetic/ABI and test scope. Workers received only
reviewed public/original project material. GPL-3.0-or-later.

The [canonical receipt](../../docs/development/pc286-floppy-provider-20261117.md)
owns native observations, exact identities and remaining limits. This modeled
diagnostic transport does not establish a physical BIOS driver, Macintosh GCR
handling, historical filesystem compatibility or System Disk boot.
