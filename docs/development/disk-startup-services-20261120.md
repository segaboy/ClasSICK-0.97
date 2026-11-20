<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Disk-startup ROM services and 286 instructions, 2026-11-20

Two independent Windows-native builds produced byte-identical packages.
The Mac ROM returned from native GetTrapAddress and SetTrapAddress calls,
ran a guest-installed RAM handler and accepted direct calls to saved ROM
routine addresses. The installed 286 ran the same guest RAM trap and both
new instruction workloads. One installation and 26 generated cold starts
passed. These are developer artifacts built without input media or demo
payloads. [Build and test commands](disk-startup-build-20261119.md)

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| `disk-startup-mac128k.rom` | 65,536 (4,788 used) | `e82529c99029226a4094e3633b5e49d8d591681ed3bae7bfd0d87d14cd774241` |
| `disk-startup-286-installer.img` | 1,228,800 | `b4727c0ec383babf1b1ab0cb485c05469051ad384fb32f1b249fd9d9b050601d` |
| Native 286 payload inside installer | 21,330 | `9f05080c28633465d87db06eb152cd29d9bee1c83c9401ec6b108189ea7b305e` |
| Hard disk after installation | 21,411,840 | `d71314b9d20c430bceccd3efd2d906162c9a1bec705c10bc37d93aa5986765e1` |

The qualification command used BootHush `543767ae09f39d330803c05bd34311b3b3f485e53f00ec814c54c82d9c8a329f`.
The two copies of `manifest.json` matched SHA-256
`d1d7645f7ca61741b3ded47b52bee026b70a895f4023ca322489cb79a92a2188`.
The test receipt `report.json` matched
`d7e90ed8248c2bc59342ef44f7e524e8137976158e5344e2f6a84b9b90cab997`.
The GNU linker reported the same missing assembly stack note and Watcom the
same missing linker-declared stack segment as the November 19 build.

| Generated workload | Mac 128K | Installed 286 |
| --- | --- | --- |
| GetTrapAddress for slot `$46` | Guest saved actual ROM Get routine address | Guest saved `$400000` |
| Install and query RAM handler `$8300`, call it, return | `$43535031`, 4,485,908 cycles | `$43535031`, 8,957,202 cycles |
| JSR to saved Get and Set addresses | Set routine pointer and `$8300` returned, 4,557,338 cycles | Outside this target profile |
| Immediate ALU, compare and CCR logic | `$00000006`, `$11200005`, 4,280,324 cycles | Same words, 11,494,376 cycles |
| Quick arithmetic, clear and test | `$FFFFFE00`, `$8000FFF8`, 4,248,700 cycles | Same words, 10,528,992 cycles |
| Uninstalled OS service, Toolbox or reserved TST form | Common ROM stop | POST `$D2` |
| Immediate write to SR | Outside this test | POST `$D2` |

Both targets also passed the original generated disk variations, transient
retry, invalid boot header and persistent read error. The CPU programs use
conditional branches over ILLEGAL to check intermediate flags, then place
their completion byte through an address register advanced by the tested
instructions. The refusal fixtures first set an entry marker; a fallthrough
would replace it. The report records each disk identity, observed bytes,
terminal PC and cycle count. Product startup contains no marker writes.

This extends SPEC-0141 / IMPL-0171 / TEST-0232 with [the immediate ALU](../specifications/SPEC-0142-immediate-alu.md),
[the Mac register traps](../specifications/SPEC-0143-mac-native-trap-address.md)
and SPEC-0145's quick arithmetic, CLR and TST. The comparison uses original
generated sectors and modeled hardware. NewPtr, Toolbox services, successful
System/Finder startup and physical compatibility remain open at this build.
