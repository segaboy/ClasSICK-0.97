<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Bounded address and stack instruction increment

2026-11-18. Canonical receipt for SRC-0616 / SPEC-0118 / IMPL-0148 /
TEST-0208. This is original shared-interpreter work supporting the selected
System Disk milestone. It does not establish System startup or execution of
any protected program. No protected executable, owner disk, reference source,
held UART body or untracked research original was used.

## Contract and implementation

`core/cpu/m68k-bus.c` now implements LEA, PEA, LINK.W and UNLK from the
lead-reviewed published instruction facts in SRC-0616 and the existing mapped
memory, control-address and guarded transaction contracts. LEA/PEA reuse all
seven existing control-address forms. LINK.W/UNLK accept A0 through A6; A7
aliases are explicitly unsupported. SR is preserved. Computed address values
are not dereferenced; all actual stack accesses use the existing complete-span
checks and guard events. Writes remain staged until final bounds and COMMIT
checks pass. The specification records checked rather than wrapping address
addition as project policy. No MOVEM, DBcc or later-CPU LINK.L was added.

Native C16 qualification exposed the existing C11-only alignment query in the
guard envelope. The pre-C11 path now uses owned member-offset wrappers with
compile-time power-of-two checks; C11 behavior keeps its existing query.
The envelope also refuses backing lengths that cannot fit native `size_t` or
`ptrdiff_t` before narrowing the guest length. This protects the tested small
near-object profile, not a general segmented backend. The old flat entry
points and earlier minimal `cs_m68k_step` interface retain their roles.

Codex authored implementation and original host/native fixtures. The lead
reviewed the published source intake and independently read the new handlers
and reused control/commit helpers; no concrete defect was identified in that
review. Worker/model-specific usage and active-effort measurements were not
provided. Human provenance review and physical hardware gates remain open.

## Exact qualification

Final successful run: `C:/Repos/.task-temp/cpu-startup-20261118-c/report.json`,
written 2026-11-18 22:36:00 UTC. SHA-256:
`32090ad6c0f1ec0f6cd4f2de7e22ce6d80ff6a9f959e5d1ed6c38d1702a4d9d6`.
The report owns complete command lines, exact declared source/tool digests,
target output, individual compiler frame reports and build identities. Inputs
were hashed before building and checked unchanged after all cases. Raw logs,
objects, original synthetic floppy/ROM probes and mutants remain outside Git
in that output directory. A byte-identical `qualified-m68k-bus.c` snapshot was
preserved there before handover for subsequent banked-memory work.

| Qualified input | SHA-256 |
| --- | --- |
| `core/cpu/m68k-bus.c` | `d9d94b61dc308502dd8fdfc7f2df0cfcb8436ffc04469679cfb1965b6de3d731` |
| `tests/m68k-startup.c` | `1bf90d3922cecf2a3856b67e763f3e946fa303622a394c6896ef432c0696cc04` |
| `probes/cpu/m68k-startup.c` | `5b7ffc5e59d395133ff18885dadf4e9bf1cfd3ebd0f05a9793b92c560792ab7f` |
| `scripts/Test-M68kStartup.py` | `461238b53bba0c22840b268f12b1276096c214b15a66b4cbeaaa4c4e1951b6fc` |
| `SPEC-0118` | `db0159c444409f5d7c7fb6a5d568da390caab459576fe632bb622cd502299625` |
| Windows Clang executable | `93303edd5791e4b60682f819267a34c349e5f52d052d067e006e76d21cc82ea4` |
| Windows m68k GCC executable | `35e82461e0cfd0e4d89be95920546155c28aabbd88227dec941c5027d1dfbe30` |
| C16 toolchain lock | `eb4b93b16d3343f37273d334bae4af4660af45317308d7a39d948fe2186f2f59` |
| Headless BootHush executable | `7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a` |

The m68k frontend reports `m68k-unknown-elf-gcc.exe (NeoGeo SDK toolchain 3.0)
16.2.0`; its Windows target/tool identities were checked by the existing
Windows-native helper. The pinned Watcom tools and headers were checked against
each selected member of the existing lock. No WSL or other emulator was used.

## Results

Thirty host sub-suites passed: the new startup suite, six existing bus suites
and five existing guard suites under both Release and address/undefined
sanitizers; the new startup and five guard suites also passed C99 builds.
The new startup suite evaluated 27,589 assertions in each of its three builds.
Cases include all admitted control modes and destination registers, all 32 CCR
combinations, exact stack bytes/canaries, extension gaps, signed boundary
arithmetic, disallowed modes, A7 aliases, permissions and all applicable guard
denials. They specifically check old A7 as the value pushed by PEA (A7), a
negative absolute-word address as a value, LEA A7 final bounds, LINK rejection
after staging a push and UNLK final rejection after reading the saved frame.

The native original program composes LINK, LEA, PEA, existing MOVE and UNLK.
Additional native cases check address-only calculation, CCR preservation,
atomic failure, final COMMIT denial, unsupported aliases and C16 native-span
refusal. Both native models distinguish three deliberate core mutations from
the baseline; the mutants are generated only in the temporary output tree.

| Model/profile | Expected marker | Result | Raw payload bytes | Modeled cycles to marker |
| --- | --- | --- | ---: | ---: |
| 286 small baseline | B8 | pass | 10,020 | 487,432 |
| 286 PEA wrong old-SP value | E2 | detected | 10,028 | 260,758 |
| 286 UNLK wrong stack increment | E3 | detected | 10,020 | 370,970 |
| 286 LINK premature write | E4 | detected | 10,050 | 417,359 |
| Mac128K O0 baseline | B8 | pass | 16,440 | 1,276,014 |
| Mac128K Os baseline | B8 | pass | 9,740 | 511,776 |
| Mac128K PEA wrong old-SP value | E2 | detected | 9,744 | 197,550 |
| Mac128K UNLK wrong stack increment | E3 | detected | 9,740 | 352,862 |
| Mac128K LINK premature write | E4 | detected | 9,760 | 409,954 |

The cycle counts include the modeled machine/loader/probe path and are not
instruction timing or physical performance measurements. Both Mac baselines
reach diagnostic address 0xB00 with B8 and a zero fault field. The 286 cases
reach the expected port-80 marker with CS=0x1000; the complete loaded payload
matches the built original bytes. Every input image hash remains unchanged.

All five C16 closure objects were compiled and import-audited in small and
compact models as USE16/80286. The bus object is 5,545 text bytes in small and
6,572 in compact, with no mutable segment bytes and only its four declared
project imports. Native execution uses the small model; compact has compile
and import evidence only. The baseline 286 payload has 2,470 BSS bytes. Both
Mac links have no unresolved imports or runtime library dependency. The O0
new handler sizes are 262 and 488 bytes; Os inlines them into the 3,014-byte
step function. Largest reported individual Mac frames are 180 bytes at O0 and
220 bytes at Os; these are not cumulative stack bounds.

Preliminary harness attempts `cpu-startup-20261118-a` and `-b` stopped on test
harness setup errors: a missing existing memory-probe link input and omitted
existing suite arguments. Those runs make no native qualification claim. The
final `-c` run corrects both and completes every declared check. No CPU test
failure was suppressed and no protected/reference input was introduced.

The qualified shared file was explicitly handed to the banked-memory worker
only after the complete unchanged-input run. Any later decoder/backend edit
requires its own qualification; this receipt continues to identify the exact
snapshot above. Root owns central CMake/provenance integration. Full CPU,
exception/timing compatibility, general segmented memory, System Disk startup,
persistent file operations and physical qualification remain separate work.
