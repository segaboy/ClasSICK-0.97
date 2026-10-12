# IMPL-0022 - Original Macintosh 128K ROM demonstrator

2026-10-11, SPEC-0018 / ADR-0021 / SRC-0076-0082 / TEST-0026.
Codex authored the original C/68000 assembly, linker layout, fixtures and build
tools from the written startup contract and published hardware interfaces.
Human provenance review pending; no independent/legal clean-room certification.

Paths: `arch/m68k/mac-reset.S`, `mac-rom.ld`, `runtime.c/.h`;
`platform/mac128k/entry.c`, `loop.c`, `rom.h`; `apps/rom-scene/scene.c`;
`tests/mac-rom.c`; `scripts/Setup-M68k.py`, `Build-MacROM.py`,
`Verify-MacROM.ps1`, `m68k-toolchain-lock.json`. Existing portable surface code
is reused unchanged. Public guard now admits original Python source files.

Design: reset executes our code at the normal ROM base, clears the overlay
before using the reset-provided RAM stack, installs own vectors/data/BSS,
checks the complete loaded image and executes one own TRAP #0 probe. An
injected-register polling module drives VBL progress and active-low mouse
edges; the native leaf performs even-addressed byte I/O only. Original tiny
lettering/geometric pixels draw through the existing MONO1_MSB surface.
Arithmetic uses independently written bounded shift/add/divide loops, and
non-overlapping memcpy; no libgcc/libc target runtime or copied font/asset.

Permitted inputs: project contracts/code, published Synertek register reference,
published retail Macintosh hardware prose/factual numeric summary, Motorola
CPU prose, compiler interface documentation, publisher tool/emulator metadata.
A sanitized private discovery audit records incidental excluded search excerpts,
no function body or affected code, and the owner's continuation strictly from
published manuals; those excerpts are not implementation inputs. No emulator
source, Apple ROM/code, confidential draft or third-party algorithm was adopted.

Build/hosted checks pass in [the snapshot](../../docs/development/mac-rom-evidence.md).
At this checkpoint the image is unloaded and Snow has not been run. Full OS
services, keyboard/floppy/Toolbox, historical parity, physical validation and
complete 128K budgets remain open. PC compiled paths and B2 criterion 6 unchanged.

Follow-up: SRC-0084 records the first own-scene startup and advancing VBL.
The original multiply loop now stops on zero remaining multiplier bits to reduce
redraw cost, retaining modulo arithmetic and at most 32 iterations. Hosted and
unloaded checks are repeated before the corrected image's qualification. The
Windows verification script also clears the expected fault child's status only
after recording/checking it, so passing controls do not falsely fail CI.

The scene now validates through the existing surface constructor and fills its
bounded rectangles with original per-row byte masks, avoiding per-pixel address
multiplication. Shared surface source stays unchanged. Complete-frame baselines
were generated before the new algorithm using the original pixel implementation;
no emulator or Apple pixels enter the fixture. TEST-0026 repeats native builds
and hosted pixel/boundary checks before another startup trial.
