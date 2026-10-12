# Macintosh 128K ROM preparation - 2026-10-11

SPEC-0018 / ADR-0021 / IMPL-0022 / TEST-0026; SRC-0076-0082.
**Original ROM built and audited; emulator trial not yet run.** This image
contains startup, an original display test and mouse polling, not System 0.97's
Toolbox, storage, desktop or applications. All full edition/physical gates remain open.

This is the retained initial preparation checkpoint. The subsequent optimized
image and passing scoped emulator/operator result are in the separate
[2026-10-12 trial record](mac-rom-trial.md); do not substitute its measurements
into this earlier build table.

| Profile | ROM file | Used ROM | Data/BSS | SHA-256 |
| --- | --- | --- | --- | --- |
| GCC 13.3 Os | 65,536 bytes | 3,168 bytes | 28 bytes | `563b98400615342f841fb11f0a4820127c605b6536ca2ad713c5511fce4d75f4` |
| GCC 13.3 O0 | 65,536 bytes | 4,588 bytes | 28 bytes | `fa3d24b4e9d9a6866f444ec36e0c3c41cd28c3d4c6561305df2d1ffb4b1e51aa` |

Both fresh twins match their optimization profile. The Os image seal is
`7ec17eaa`, O0 `2c0c31a3`, over the entire padded image with the four-byte seal
field zeroed. The build links only our six source modules; 68000 assembly,
big-endian reset/vector layout, entry and RAM bounds, helper/import closure,
bounded stack reports, seven corruption controls and six omitted-object controls
pass for every build. Existing portable surface bytes remain unchanged.

24 hosted suites and four zero-divisor controls pass four Clang/GCC profiles.
Each arithmetic suite compares 100,000 pairs with independent host-width
oracles; injected device traces exercise flag acknowledgment, stalls, mouse
edges/holds and completion saturation. Hosted checks do not execute hardware.

Candidate Snow v1.5.0 Windows executable identity/black-box clearance is SRC-0082.
The archive contains only its executable; no Apple ROM, disk or emulator source
was downloaded. SRC-0076's owner continuation authorizes the isolated own-image
trial after these checks. No first-boot claim is made at this checkpoint.

Retained preparation issues: archive catalog request denied; direct NXP download
denied (cached publisher prose reviewed instead); an initial WSL inline-command
quoting error was replaced by a saved script; missing compiler libraries were
added only to the workspace copy; GCC bounded argument pushes initially tripped
a static-only stack checker, now correctly bounded; Clang rejects a GCC-only
optimization switch, so the hosted verifier applies it only to GCC. None involved
guest execution or prohibited input. Source discovery exposure is documented
privately; no uncertain result is an implementation input. Human review pending.

Reproduce in Ubuntu 24.04: run `scripts/Setup-M68k.py --output <tools>`, then
`scripts/Build-MacROM.py --tool-root <tools>/root --output <fresh-build>`.
On the prepared Windows developer workstation run
`scripts/Verify-MacROM.ps1 -BuildRoot <fresh-output>`. Neither script launches an
emulator. The independent Linux/Windows publication checks will be recorded
when observed; existing PC CI is not evidence of Macintosh execution.
