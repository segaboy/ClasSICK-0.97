<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0141 - Build the disk-startup developer ROM and installer

The November 20 build adds [SPEC-0143's native register traps](SPEC-0143-mac-native-trap-address.md)
and the 286 instruction support in SPEC-0142/0145. The Mac's Line-A
terminal-only behavior below describes the November 19 profile. See the
[subsequent generated cold starts](../development/disk-startup-services-20261120.md).

The Windows-native builder takes the m68k tool directory, 286 toolchain,
assembler and a new output directory. Build both products from explicit
project source lists without a disk, emulator, installed environment or
preview payload as input. Deliver a 65,536-byte image, SPEC-0110 installer
floppy and machine-readable manifest. Record the linked sources, transitive
local headers, tool identities, artifact hashes, vectors, native static
storage, stack placement, and expected installed-disk identity. Publish the
package only after both products link and pass their static layout checks.

On the Macintosh, release the reset overlay and install the 64-entry
vector table. Initialize the C data/BSS spans and the SPEC-0137 globals.
Read and validate the two startup blocks through the internal IWM. Retry
one checksum rejection of a sector. With interrupts masked, transfer
control to $8002 with A0=$8000, A7=$7800, SR=$2700 and the remaining
address and data registers zero. These values define an experimental entry
contract. A failed load or unsupported exception reaches a terminal stop.

On the installed 286, boot from drive 80h using the fixed whole-disk profile
and read the Macintosh logical sectors from drive 00h. Set up the same
guest entry state in 128 KiB of banked RAM and step its 68k program. Admit
the decoder and OS trap cases supported by the linked backend. Native
Get/Set tokens are $400000/$400002. Unimplemented slots use $400020. The
directory admits even ROM addresses $400000 through $40FFFE and even RAM
handlers $001000 through $010FFE. SPEC-0135 defines which handlers can run.
A load failure halts at POST D0h, setup at D1h, and a refused CPU/service
step at D2h. The runtime does not impose the diagnostic harness's step
limit, boot-image fence, or manufactured return address.

R2/R8 remain open. This is a developer startup profile for modeled hardware.
The Mac Line-A vector stops, the 286 CPU and service interfaces are partial,
and the combined heap, application footprint, chip mapping and physical
hardware workload require qualification. Existing preview release artifacts
and the independent diagnostic fixtures keep their prior roles.
