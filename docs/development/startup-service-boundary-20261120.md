<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# November 20 System Disk startup trials

The [November 19 normal startup trial](installed-disk-startup-20261119.md)
reached the replacement Mac ROM's Line-A vector after 38 steps in the two
loaded sectors. The installed 286 entered its guest dispatcher three times,
then emitted the runtime-failure POST. This trial uses the same read-only
DC42 container, SHA-256
`1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647`.

## Standalone ROM and installed 286

The twin builds passed 26 generated cold boots. An independent generated
control then reached the Mac ROM's GetTrapAddress and RTE entries and made
four 286 dispatcher visits. The [trial runner](../../scripts/Test-StartupServiceBoundary.py)
installed the runtime on a fresh fixed disk and ran three identical read-only
repetitions with the separate supplied floppy. The source media, installed
disk, twin artifacts, manifest and emulator identities matched after testing.

| Observation | Macintosh 128K | Installed 286 |
| --- | --- | --- |
| Initial execution | `$8002` at 4,364,010 modeled cycles. Its first step completed at `$808A`. | Native payload entry at 3,333,596 cycles. |
| Boundary | 38 of 38 in-range steps completed; the last started at `$80EA`. The ROM entered vector 10 at `$4009F0`, cycle 4,364,568. | The observer counted three visits to the linked dispatcher, first at 7,201,538 cycles and last at 7,521,883. |
| Terminal behavior | The ROM reached its common stop at `$400AB2`, cycle 4,364,964. GetTrapAddress, SetTrapAddress and the successful service return were unreached at the 16-million-cycle horizon. | The third dispatch failed. A separate replay reached POST `$D2` at 7,634,636 cycles. The normal runtime provides no guest PC or detailed reason at that stop. |

Neither target reached System or Finder. The semantic diagnostic below
identifies the missing Mac service. The CPU diagnostic identifies the
remaining 286 guest-PC boundary.

| Standalone artifact or receipt | SHA-256 |
| --- | --- |
| BootHush executable | `543767ae09f39d330803c05bd34311b3b3f485e53f00ec814c54c82d9c8a329f` |
| Mac ROM | `e82529c99029226a4094e3633b5e49d8d591681ed3bae7bfd0d87d14cd774241` |
| Native 286 installer | `b4727c0ec383babf1b1ab0cb485c05469051ad384fb32f1b249fd9d9b050601d` |
| Installed hard disk | `d71314b9d20c430bceccd3efd2d906162c9a1bec705c10bc37d93aa5986765e1` |
| Manifest | `d1d7645f7ca61741b3ded47b52bee026b70a895f4023ca322489cb79a92a2188` |
| `startup-developer-verify-20261120-d/report.json` | `d7e90ed8248c2bc59342ef44f7e524e8137976158e5344e2f6a84b9b90cab997` |
| `startup-service-boundary-selected-20261120-a/report.json` | `0cacf9414b4dccd0ae4819bdce8bea99bd15ccfc897234d8888879bc9def1e2c` |

## First Macintosh service request

A separately built Macintosh diagnostic routed the first call into a catalog
of named public service entries. It reached **NewPtr** in three cold starts.
That result identifies the disk's first requested service in this startup
profile. The allocation size, options and result were not observed. Sixty
authored controls validated the classifier, including unknown selectors and
startup without a service request. See the [semantic diagnostic receipt](startup-service-labels-20261120.md)
for the catalog, artifact identities and qualification limits.

## Bounded CPU diagnostic

The [SPEC-0133 harness](../../scripts/Test-BootHandoff.py) was rebuilt with
the expanded immediate decoder, SHA-256
`662a9d4aa39b9e6fcd5d033d36baf76ca10fbb662e98732faeabddb9372e0d16`.
All eight generated controls passed. Two cold 286 trials reproduced the
previous result. The first step transferred from `$8002` to `$808A`; two
steps completed before the trial returned its unsupported-instruction status
at guest PC `$808E`. That status identifies the runtime's refusal category.
The failing instruction remains unidentified. This test retained the
November 19 emulator, generated ROM, stack return sentinel, 256-step limit
and boot-region fence to compare the decoder in isolation.

| Diagnostic artifact or receipt | SHA-256 |
| --- | --- |
| BootHush executable | `a00fb8b507d79b0f0b88026c2e9cd613a6a7bd78c3aae10858bb5735e4f6db98` |
| Mac diagnostic ROM | `fe835dcf1e4468928c316332ffb1f282ba0fa62ed8cd40699a175636c578aaa3` |
| 286 diagnostic payload, 17,602 bytes | `336ebe4b7aab4bd9657b45ebc8c5bb6bc9674548252a53d2fc75fa96dae3af9a` |
| Installed diagnostic hard disk | `69599078dbc6ed9bd2914390f307c6c96df385ba297e81cba04948ab8d50e340` |
| `boot-handoff-decoder-20261120-a/report.json` | `c14ec52de6d29c78957a370c53e541371c74e42f208bb0923651f8bd526b33f7` |
| `boot-handoff-decoder-selected-20261120-a/report.json` | `e9f2cfbc4161acc750dc2ded8eed4516c66ec418dc3aea9536bff70e5dcb1a6d` |

A second build added immediate, quick ADD/SUB, CLR and TST support. All
eight generated controls passed again. Both cold repetitions on the supplied
disk still completed two steps and returned the same project status at
`$808E`.

| Second diagnostic artifact or receipt | SHA-256 |
| --- | --- |
| Shared decoder | `6dec13a53da91f0f4095a661d28708637a72a1c444fa35fffe8d275a66c3a431` |
| 286 diagnostic payload, 18,134 bytes | `b5efeacbe0218804a054a73637edb1c5cba395bbf437a5b45f938485d902bc71` |
| Installed diagnostic hard disk | `43fad7a30978054a013ad2466bc0be5cee9500043de68501d228330c47767c56` |
| `boot-handoff-decoder-20261120-b/report.json` | `04cfd7108ec6268ea25192ee8a43a9c7e0c3a3575182c205a0d92b077c34a4b0` |
| `boot-handoff-decoder-selected-20261120-b/report.json` | `a569686d8d2a055a5a68bd1a9375a3b2e1882de9dfb6668811f023d710683b03` |
