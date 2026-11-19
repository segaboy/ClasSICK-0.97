<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Boot-block transfer on native Macintosh and installed 286

2026-11-19. [SPEC-0133](../specifications/SPEC-0133-boot-handoff.md) defines
one testable startup hypothesis. The Macintosh reads both sectors through the
IWM, seeds four documented low-memory variables, and branches into the RAM
copy. The installed 286 reads the same two sectors from a separate floppy,
applies the same four initial values, establishes guest CPU state, and runs
the disk program with the banked 68000 interpreter. [IMPL-0163](../../provenance/records/IMPL-0163-boot-handoff.md)
and [TEST-0224](../../provenance/records/TEST-0224-boot-handoff.md) identify
the original implementations and test inputs.

The 1985 published Apple manual specifies the two-sector read. The 1987
Mac Zap disk-tool manual, Appendix B p. 59, identifies the offset-two entry
as a branch. These accounts support testing control transfer through that
field. Neither establishes the chosen $8000 placement, SR, register values,
stack, return address, or timing/availability of globals for the original
shipping ROM. A 1992 HFS-era version policy does not resolve early MFS
dispatch. The earlier selected-disk metadata run reported version $0012.
A successful transfer here cannot authenticate that convention on stock hardware.

## Generated qualification

Both targets returned to $7FFE after three instructions in one fixture.
A second fixture used absolute-long addressing to read ScrnBase from guest
$0824 and then jumped through it. Both reached $1A700. Each program's first
branch reached $81FE. Both targets rejected the malformed signature and the
injected failure of the second sector. Each PC case cold-booted the installed
runtime with the installer removed. The original blank and full installed
images were verified around the installer run.

The first generated test also exposed a gap using absolute-short addressing
for the ScrnBase operand. The native Mac reached the expected target, while
the initial 286 MOVE implementation refused this addressing mode after one
successful instruction. That observation came entirely from generated code.

| Generated qualification | SHA-256 |
| --- | --- |
| BootHush including RAM-step observation | `c709c7f10b5f8e2a908cc2f06a74192269073dda52e8aa4f7d23a80dac89ac28` |
| Mac diagnostic ROM, 2,972 linked bytes | `55ca166991dad53e2fe423d36c6abb1b85faf1bd5b5eacbcc912cd51679c56e6` |
| Original 286 runtime, 15,876 bytes | `29cd967e040590ed396c99bfc8bae1568f6a70faff6dad86fb7861e92a4e51ed` |
| Full installed disk image | `1dd7844578ca4b3f5cf616291b6b4466f10759de167ad0cbbbc13a8e79da64cb` |
| `boot-handoff-20261119-b/report.json` | `7f5d8f9f389d226aaabb699f9c194469c319b4b4775b51e08da716a10b7a00e9` |

Run [Test-BootHandoff.py](../../scripts/Test-BootHandoff.py) with the pinned
emulator, Windows-native m68k and 286 toolchains and a new external output
directory. Public mode creates and verifies the original installer, target,
fixtures, and their identities before allowing a separate private invocation.
The private path accepts only the identified, unchanged System Disk, pads its
unmodified logical sectors into a temporary read-only 286 carrier, and arranges
that both disk inputs and the installed runtime remain unchanged. Its output
is a numeric report. The diagnostic returns no code, raw registers or images.

## Supplied System Disk

Two cold trials of System Disk 690-5003-C, container SHA-256
`1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647`,
produced identical numeric results. The unchanged container supplied all 800
logical sectors. Each target read its two startup blocks through the modeled
drive and entered at $8002. The first completed step on each reached $808A.

On Mac the run reached the entry after 4,388,362 modeled cycles and completed
the first step at 4,388,372. In the bounded longer run the monitor counted eight
started steps inside the image, seven returned. The last started at $809E. The
emulator reported PC $80A4 at 4,388,432 cycles and the 32-step condition was
unmet. The original 286 runtime completed one step and refused the next at
$808A with the interpreter's unsupported-opcode status. The return value
does not identify an instruction. Neither the identity nor the cause of any
private instruction path was inferred from the generated-control finding.
The System file, Finder and desktop did not reach a verified running state.

The retained private numeric report is
`boot-handoff-selected-20261119-a/report.json`, SHA-256
`97a6811d85f912df3a1163753c90730f0887245f09cbde0a6b596fd33b8ac29c`.

## Qualified image-boundary and decoder corrections

Review found that the first Mac probe placed a native structure directly at
$8000. Its decoded header followed the two sectors at $8400, whereas the PC
published just the two sectors there. Both probes now keep their metadata in
native storage and copy only the two sectors to guest $8000. The generated
tests check that the next 16 bytes remain zero. The tested Mac entry state
includes all D/A registers, SR and the return slot. The PC checks its own
frame and the mapped return slot before stepping.

Seven generated cases passed on both targets in the final build. The absolute
short and long forms now both reach the seeded ScrnBase pointer. A direct jump
to $7FFE and an RTS reach the same boundary; the installed 286 distinguishes
the unchanged A7 from the popped slot. A generated write to $800000 raises
the expected mapped-model write8 fault. The read-failure control checks the
actual target of injection on each machine.

Two fresh System Disk trials on the corrected builds again agreed. The Mac
reached $8002 at cycle 4,471,488 and $808A at 4,471,498. It started eight
in-range steps, completed seven, and reported a modeled **read16 fault at
$03FFF0** after starting at $809E, stopped PC $80A4, cycle 4,471,558. This is
outside the installed 128 KiB primary RAM extent. The hardware response at
that logical address remains unverified. The native 286 completed **two**
steps and refused at **$808E** with unsupported-opcode status. The harness
records no private opcode or register contents. Startup ABI fidelity, physical
hardware behavior, System/Finder boot and further services remain open.

| Final artifact | SHA-256 |
| --- | --- |
| Mac diagnostic ROM, 2,992 linked bytes | `fe835dcf1e4468928c316332ffb1f282ba0fa62ed8cd40699a175636c578aaa3` |
| Installed 286 runtime, 16,088 bytes | `7e084fd991ee46d31c9adfd06057719c9dacb28304532f07b6ee7f9306abe906` |
| Entire installed disk | `6e1e389936e47031921484735f7bc9eb90c733c5d17321078aee69dcbbd3ae98` |
| `boot-handoff-20261119-e/report.json` | `680114f4d686f839600b9b8616718fdc1f785147a1b4257cdf8701ff7390bac3` |
| `boot-handoff-selected-20261119-d/report.json` | `7ae34929e3ef2086dea2c5db868e9b6a3a6eeef282f0f4fb679114d0f949de6a` |
