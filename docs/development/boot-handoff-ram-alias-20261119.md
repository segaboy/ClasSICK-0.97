<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# System Disk reaches the replacement ROM's Line-A handler

2026-11-19. The same 64 KiB Mac test ROM that previously stopped after seven
completed startup steps now completes 38 steps inside the System Disk's loaded
startup image. The previous BootHush model faulted on a read at $03FFF0.
The updated model maps that address to physical RAM at $01FFF0. The replacement
ROM receives control at its own linked Line-A exception handler. The installed
286 still stops at logical PC $808E after two completed steps. These are
observations on the existing provisional SPEC-0133 startup convention.

[Earlier trial at ClasSICK commit 25bacc6](boot-handoff-20261119.md) and
[128 KiB RAM aperture contract](../specifications/SPEC-0139-mac128k-ram-aperture.md).
Apple's published 1985 Inside Macintosh III-17 and III-45 supply the RAM
wrapping basis. Both models in this trial use one physical 128 KiB RAM
array after ROM overlay release. The new 286 runtime also includes the
current SPEC-0140 displacement/index addressing work. Its observed two-step
stop remains unchanged; the unsupported private instruction was not inspected.

## Controls

All seven earlier authored startup, refusal and fault controls passed. An
eighth original program wrote values through $03FFF0, $3FFFF4 and $3FFFFE,
the last aligned word below the upper aperture edge. It read the values back
through different logical aliases. Fourteen disk-loaded instructions returned
through the prepared stack slot on both models. External inspection compared
twenty physical bytes including the surrounding guards, plus a separate
ten-byte readback. On Mac these ranges began at physical $1FFEC and $9100;
the corresponding installed 286 bank backing began at $4FFEC and $39100.
The unchanged unmapped-write control still targets $800000, above the
mirrored RAM aperture.

## Selected trial

Container 690-5003-C, SHA-256
`1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647`,
remained unchanged. Two cold repetitions returned identical numeric reports.
The test harness captured neither private opcodes nor any guest register or
RAM contents on these runs.

| Observation | Mac 128K | Installed 286 |
| --- | --- | --- |
| First completed disk step | $8002 to $808A, 4,471,498 modeled cycles | $8002 to $808A |
| Later observation | 32 completed at $80DC, 4,471,924 cycles | Two completed, unsupported-opcode status at $808E |
| First ROM-service boundary observed | Independent replay stopped at owned symbol `cs_mac_line_a_handler`, $400B24, cycle 4,472,046 | No corresponding boundary reached |
| Extended run | 38 steps started and completed in the 1024-byte disk image; last step started at $80EA. Stopped inside the ROM handler at $400B2E at the 16,000,004-cycle limit. | No further execution after interpreter refusal |

The Line-A handler in this diagnostic ROM parks the machine. This result
identifies where service dispatch is needed. It does not establish a trap
selector, a service return, a System or Finder boot, or physical-machine
compatibility.

| Artifact | SHA-256 |
| --- | --- |
| BootHush with 128K RAM wrapping | `a00fb8b507d79b0f0b88026c2e9cd613a6a7bd78c3aae10858bb5735e4f6db98` |
| Unchanged 64 KiB Mac ROM, 2,992 linked bytes | `fe835dcf1e4468928c316332ffb1f282ba0fa62ed8cd40699a175636c578aaa3` |
| Installed 286 runtime, 16,808 bytes | `97a538e8d73cb035332a4f7a7b37aacf1831ace492f96ec79059040dea412d5f` |
| Complete 286 hard disk | `4553cd42f03514d84394cdd1f68b15c227878c3f72d190fe4726b50df3457cf5` |
| `boot-handoff-alias-20261119-c/report.json` | `099e9ce6e96b21441ed20bd34a03381328a79919905831331b5889fd4ad80c6e` |
| `boot-handoff-alias-selected-20261119-c/report.json` | `4fe0694d8a4c33262558c6f24f04a5a476c99aad912033e3b61e9f02fdac2e31` |

Reproduce with the current [handoff runner](../../scripts/Test-BootHandoff.py),
first in its generated mode, then with its admitted read-only disk and the
completed build receipt. To reproduce the prior strict-RAM trial use the
runner and native sources from commit `25bacc6` with the exact prior BootHush
binary recorded in the earlier receipt.
