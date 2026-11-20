<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Native trap-address services in the Macintosh disk-startup ROM

On 2026-11-20 the normal 64 KiB replacement ROM booted three independently
authored cases in the BootHush Macintosh 128K model. On the positive disk,
the 68000 took vector 10, the guest obtained the actual GetTrapAddress and
SetTrapAddress entries at $400A76 and $400A94, and the guest called those
addresses directly. It installed a RAM handler, invoked it with each
register-based OS return convention and invoked GetTrapAddress inside the
handler. The return from the mediated call passed through RTE at $400A74.

The guest recorded both native addresses, CCR values, preserved D1/D2/A1,
the handler's D3 result and the requested A0 return/suppression in ordinary
RAM. A7 was $7800 before and after. The final success marker was $BC. The
unsupported Toolbox call and the odd-address Set each wrote a pre-trap
sentinel of $55, reached the ROM's designated terminal handler, and left
the post-call success marker clear. No Apple code, disk or hardware was
used in these tests.

| Cold boot | Modeled cycles |
| --- | ---: |
| Register ABI, direct calls and nested patch | 4,580,692 |
| Unsupported Toolbox trap | 4,687,386 |
| Odd patch address | 4,596,240 |

Run `scripts/Test-MacStartupTraps.py` with `--product` pointing at a completed
normal disk-startup build, `--m68k-bin` at its GNU cross-assembler directory,
`--emulator` at the BootHush executable, and `--output` at an empty directory.
The accepted report is
`C:/Repos/.task-temp/mac-startup-traps-20261120-c/report.json`, SHA-256
`858550969b9cd2429af9e12426ab0498cb32fda63f56eb19b278e50151af50b2`.
The ROM SHA-256 is
`e82529c99029226a4094e3633b5e49d8d591681ed3bae7bfd0d87d14cd774241`.
The report includes the fixture, assembler output, product manifest, emulator
and disk hashes, the complete observed RAM values and the cycle counts.
This qualifies the supervisor-mode services specified in SPEC-0143.
