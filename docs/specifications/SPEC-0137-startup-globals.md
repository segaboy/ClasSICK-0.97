<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0137 - Four startup pointers on the 128 KiB guest

Prepare the following pointer values in a writable contiguous guest RAM
window anchored at $0100. Offsets and meanings follow SRC-0624.

| Pointer | Global address | Guest pointer value |
| --- | ---: | ---: |
| MemTop | $0108 | $00020000 |
| SoundBase | $0266 | $0001FD00 |
| ROMBase | $02AE | $00400000 |
| ScrnBase | $0824 | $0001A700 |

Each store changes exactly four bytes in Macintosh byte order. Reject null
and windows shorter than $728 bytes without modifying memory. On the PC the
caller supplies the corresponding guest RAM backing. The ROM pointer is a
guest address, and publishing it alone provides no services at that address.
Call before guest execution with exclusive access. Interrupt-visible atomicity
and original pre-entry initialization order are outside this contract.
