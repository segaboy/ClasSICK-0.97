<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Build and boot the disk-startup developer targets

This Windows-native build produces a 64 KiB developer ROM for the Macintosh
128K and a bootable installer floppy for the 286. Each target reads its
startup program from a separate Macintosh disk when it boots. The build
requires only the checkout, Python, and the pinned compilers and assembler.
The existing preview build commands continue to produce the surface demo.

## Build

From the repository root, set `$M68kBin` to the Windows-native m68k tool
directory, `$ToolRoot` to the Open Watcom tree containing `binnt64` and `h`,
and `$Wasm` to the pinned `wasm.exe`. Choose a new directory outside the
checkout for each command.

```powershell
python -B scripts/Build-DiskStartup.py --m68k-bin $M68kBin --tool-root $ToolRoot --assembler $Wasm --output C:/build/classick-startup
```

The output directory contains exactly three files. `manifest.json` records
source and tool identities, both linked source lists, the Mac vectors and
static storage, and the native entry and dispatcher physical PCs. The
builder stages both products before publishing them. An attempt to build
over the qualified output was rejected and all three file hashes remained
unchanged.

| Output | Bytes | Qualified SHA-256 |
| --- | ---: | --- |
| `disk-startup-mac128k.rom` | 65,536 | `134962b21fdaf4c84efaf73b293d0469854de25dac58be8b877c6321f54973f2` |
| `disk-startup-286-installer.img` | 1,228,800 | `78b1f8ef075fba24bf0d730f4956c6385bb7eff34471e60ea7b205e762da04bc` |
| `manifest.json` | 8,940 | `70d4c68ed86c70a4b7efaff88414eb3739e34afd7a7cc747c8e7957705f31b3f` |

The linked Mac image uses 3,012 bytes, with `$FF` padding to 64 KiB. Its
reset vector is `$400440`, with initial SSP `$4000`. The linked data and
BSS use the RAM interval `$0B00` through `$1870` (exclusive). This includes
706 IWM wire bytes and 524 decoded bytes. The sum of the compiler's C frame
sizes is 276 bytes. Link-time checks reserve `$2000` through `$4000` for
the startup stack. The assembly reset path gives the disk program its own
stack at `$7800`. The linker reported a missing GNU-stack note in assembly.

The 286 image carries a 20,004-byte native payload, SHA-256
`f686a8b151f0263a28a03260037f6be957f601810f76361d8d27021ac91f5acb`,
followed in memory by 3,466 bytes of BSS. Its loader sets the native stack
top to `$FFFE` within the segment. Watcom reported W1014 for the absence
of a linker-declared stack segment. The banked guest's 68k stack starts at
`$7800`. The install operation described below exercised the loader and
native entry on a freshly initialized disk.

## Generated-disk verification

Use the independently pinned BootHush Windows executable with SHA-256
`a00fb8b507d79b0f0b88026c2e9cd613a6a7bd78c3aae10858bb5735e4f6db98`.

```powershell
python -B scripts/Test-DiskStartupBuild.py --m68k-bin $M68kBin --tool-root $ToolRoot --assembler $Wasm --emulator $BootHush --output C:/build/classick-startup-verification
```

The test produced identical independent builds, entered `INSTALL` into
the project installer, and verified the complete 21,411,840-byte installed
disk against SHA-256 `299cb6960b5e15cf3d25b929c3dd072998c14eed8059d35ba28281ac4f02b665`.
It then cold-booted those products twelve times. The test generates all
disk bytes itself. It supplies a 400 KiB logical disk to the Mac and pads
that same logical data to a 1.2 MB transport for the 286, with the fixed
disk selected first. Installer and transport hashes were stable. The test
recorded `generated-controls-pass`. The resulting `report.json` SHA-256
was `afb869d635afd76bc4a6e9691d432a0ae614c3b5354d38ef6527a2a2a063af87`.

| Generated disk or injected condition | Mac observation (cycles) | Installed 286 observation (cycles) |
| --- | --- | --- |
| Baseline | `$37` (4,644,544) | `$37` (56,317,963) |
| Change the guest program's constant | `$82` (4,649,456) | `$82` (56,317,966) |
| One recoverable error | `$37` (7,030,278) | `$37` (56,318,149) |
| Invalid boot header | ROM stop, zero marker (4,593,280) | POST `$D0`, zero marker (5,985,890) |
| Persistent read error | ROM stop, zero marker (3,964,298) | POST `$D0`, zero marker (5,915,937) |
| Guest GetTrapAddress of slot `$46` | Outside Mac coverage | Guest saved `$00400000` (56,739,002) |
| Install, query and execute a guest RAM trap handler | Outside Mac coverage | Handler at `$8300` returned `$43535031` (8,751,535) |

The positive variations execute `DBRA` 301 times, write new 68k code at
`$8800`, and jump there. That code writes the marker observed externally
at `$9100` on Mac and at backing address `$39100` on 286. The 286 trap
controls also check guest result bytes after dispatch. The runtime has no
completion marker of its own.

The Mac retry corrupts one delivered checksum nibble of sector zero, and
the ordinary reader recovers on its second attempt. The persistent control
alters two returned nibbles and reaches the ROM failure vector at `$4004D0`.
A preliminary one-shot injection at sector one was consumed while scanning
and reached the marker with the baseline cycle count. Qualification uses
sector zero and requires both the injection count and a longer successful
boot. On the 286, the one-shot fault hits the fixed-disk payload loader;
the persistent fault targets a Macintosh source-sector BIOS read through
drive 00h. Each PC case observed the expected firmware injection.

The separate generated `Test-PC286Mac128kRam.py --transfer` control
checked each length from 1 to 64 bytes across the RAM mirror and guarded
the end of the aperture and read-only backing. Its 5,706-byte native payload
ran for 10,610,376 cycles. Its report SHA-256 was
`3a3c64f7825dc2c8b06b4f691cb2593bd84d27a5b0a59c1fd4baea1fb08559f5`.

## Compatibility boundary

The Mac Line-A vector stops, and the 286 supplies a partial CPU decoder
and limited OS trap services. The generated disks demonstrate startup
and guest execution under the modeled hardware. Full replacement-ROM
parity, the four original disk workloads, and physical machines still
require qualification. See [SPEC-0141](../specifications/SPEC-0141-disk-startup-build.md)
and the [original-ROM parity audit](original-rom-parity-20261119.md).
