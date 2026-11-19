<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Original-disk trial on the developer ROM and installed 286

Three cold repetitions with the supplied 400 KiB System Disk produced the
same observations. The Mac ran 38 steps from the two startup sectors and
entered the replacement ROM's unimplemented Line-A vector. The 286 entered
its native guest dispatcher three times, then reached its runtime-failure
stop. System and Finder startup remain unverified.

| External observation | Mac 128K | Installed 286 |
| --- | --- | --- |
| Entry | Disk PC `$8002` at 4,341,896 modeled cycles | Native payload PC `$14CF5` at 3,128,105 cycles |
| First disk step | `$8002` to `$808A` at 4,341,906 cycles | First native dispatcher entry `$14A46` at 6,996,121 cycles |
| Later execution | 38 steps started and completed in `$8000–$83FF`. The last started at `$80EA`. | The same dispatcher PC at 7,155,709 and 7,316,441 cycles. The program loop reaches the next call only after the preceding call returns success. |
| Stop | Dedicated vector 10 entry `$4004CE` at 4,342,454 cycles. Common stop `$4004D0` at 4,342,458. A bounded replay reached 16,000,000 cycles parked at `$4004D4`. | The third call reached native POST `$D2` at 7,429,063 cycles. A separate run looking for a fourth visit halted at 7,429,067 cycles. |

The Mac observation identifies a missing service entry. The trap selector
and service identity remain unknown. POST `$D2` identifies a failure returned
by the installed 286 dispatcher. The normal runtime exposes neither its guest
PC nor the reason for that return.

## Qualification and media

The [generated qualification](../../scripts/Test-DiskStartupBuild.py)
produced two identical packages and installed the 286 payload on a blank
hard disk. The [selected-disk runner](../../scripts/Test-InstalledDiskStartup.py)
checked both package and manifest identities, the current source hashes and
the BootHush executable, then independently repeated the installation to a
fresh hard disk. Every observed 286 run cold-booted that installed disk with
the installer removed. The separate floppy carried all 800 logical sectors
from the checksum-verified Disk Copy 4.2 container, followed by zero padding
for the 1.2 MB transport. The Mac used the original container directly.
All input and installed-disk hashes matched after execution. The temporary
transport and fixed disk were removed when the runner exited.

An additional authored control took a cross-sector branch and ran a MOVEQ,
GetTrapAddress and ILLEGAL sequence. The Mac model entered the dedicated
Line-A vector on its third in-range step. The installed runtime made four
visits to its linked dispatcher before emitting `$D2`. Thus the runner can
distinguish separate dispatch calls even when they share a native PC. Each
subsequent address check replays from power-on with its cycle threshold one
past the previous visit. BootHush supplied only filtered PCs, step and cycle
counts, stop classes and the project-defined native POST for the selected disk.

## Identities

| Input or receipt | SHA-256 |
| --- | --- |
| BootHush executable | `a00fb8b507d79b0f0b88026c2e9cd613a6a7bd78c3aae10858bb5735e4f6db98` |
| Supplied DC42 container | `1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647` |
| Developer Mac ROM, 65,536 bytes | `134962b21fdaf4c84efaf73b293d0469854de25dac58be8b877c6321f54973f2` |
| Developer 286 installer | `78b1f8ef075fba24bf0d730f4956c6385bb7eff34471e60ea7b205e762da04bc` |
| Installed 286 payload, 20,004 bytes | `f686a8b151f0263a28a03260037f6be957f601810f76361d8d27021ac91f5acb` |
| Entire freshly installed hard disk | `299cb6960b5e15cf3d25b929c3dd072998c14eed8059d35ba28281ac4f02b665` |
| Package manifest | `70d4c68ed86c70a4b7efaff88414eb3739e34afd7a7cc747c8e7957705f31b3f` |
| `startup-developer-verify-20261119-b/report.json` | `afb869d635afd76bc4a6e9691d432a0ae614c3b5354d38ef6527a2a2a063af87` |
| `installed-disk-startup-selected-20261119-a/report.json` | `f9aed136b34e5ff1e783bc24a274e2898e5edc1a0a640bf3ef10b77f42ef8ac9` |

The [earlier diagnostic run](boot-handoff-ram-alias-20261119.md) observed
the same 38 Mac disk steps and directly reported two completed 286 guest
steps with a refusal at guest PC `$808E`. That separate diagnostic ROM and
installed program had different binary identities and observation points.

To reproduce the normal trial, run `scripts/Test-InstalledDiskStartup.py`
with `--emulator`, the generated control directory as `--qualification`,
its report hash as `--qualification-sha256`, the admitted DC42 as `--disk`,
and a fresh external `--output`. It writes a numeric `report.json`.
