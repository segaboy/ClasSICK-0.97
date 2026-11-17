<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Owner-selected System Disk boot milestone - 2026-11-17

## Direction and exact target

SRC-0612 records the owner's new priority: make the selected Macintosh System
Disk boot against ClasSICK on both the original Macintosh and the 286. This
follows the published paired interactive-window preview; published releases and
their qualification remain immutable.

The owner identifies a disk from their collection and confirms it is a local
image. Its path, byte length, hash, container format, locale and exact version
have not yet been supplied or verified. The discussion identifies the January
1984 System Disk as the intended candidate, not as an established identity of
this file. Do not substitute another download or infer identity from a filename.

The new direction authorizes bounded, read-only BootHush interoperability trials
of that owner-selected image once its exact location and identity are recorded.
It supersedes older statements that this goal is documentation-only or that a
fresh owner go-ahead is needed for every such retry. The disk remains a private
black-box runtime input; no Apple implementation becomes a development source,
distributed dependency or public fixture. No Apple ROM is required for this lane.

## Acceptance

| Lane | Required result | Current state |
| --- | --- | --- |
| Mac128K in BootHush | Cold start with independently built ClasSICK ROM and the exact selected disk reaches the disk's usable desktop; mounted-volume access and mouse/keyboard interaction work. | No selected-disk run; guest floppy/System boot path absent. |
| Native 286 in BootHush | Native 286 ClasSICK provides the required ROM/OS services and a defined legacy-code/media bridge for the same disk payload; equivalent startup, volume and interaction workload completes. | No selected-disk run; bridge, filesystem and broad 68000 execution absent. |
| Original physical Macintosh | The independently built replacement ROM and selected disk repeat the Mac acceptance workload on identified original hardware. | Required, unverified; emulator success does not close this row. |
| Physical 286 | An identified native 286 machine repeats the selected workload using a declared media-transfer method. | Unverified; no claim from emulator behavior. |

The desktop must come from successful disk-system startup, not the built-in
ClasSICK scene or a substituted original fixture. Log each stage reached and the
first unresolved dependency; a mounted image, motor command, successful sector
read, screenshot or absence of an immediate fault is not a boot pass.

For each executable lane, record three bounded cold starts with pinned source,
ROM/native image, disk and BootHush identities; retain failures and repeatability.
Verify that the original input hash is unchanged. Keep original-fixture no-disk,
truncated-media and injected-read-failure controls separate from reference runs.
Capture useful private frames/video when a visual startup milestone is reached;
reference pixels do not enter the public evidence gallery automatically.

## Audit of the current implementation

Audit baseline: ClasSICK 0.97 dc6d823ae12e7aaa6c99b782c4724d2aef7721c8;
BootHush f7aa60b683de85b571ca01ce45966ca61f43f1c2. These are source observations,
not historical behavior or newly executed tests.

- Mac startup enters an original built-in demonstration. The separate loaded-app
  profile uses an embedded original resource fixture, not disk contents.
- The tracked MFS implementation area contains only a README. A Mac floppy
  driver, block-device layer, MFS mount, historical File Manager and System Disk
  startup coordinator are not implemented. The reviewed MFS intake retains an
  unresolved allocation-map packing question.
- The published 286 bootloader loads a fixed CHS native preview payload and checks
  its CRC. It has no filesystem. Its source list does not link the interpreter,
  Memory Manager, resource or CODE loader modules into that preview.
- Existing 68000 execution is a selected-instruction, constrained application
  profile. General System startup, callbacks, patches, interrupts and dynamic
  loading are not established; production segmented guest-memory integration on
  the 286 remains incomplete.
- BootHush already models a read-only single-sided 400 KiB Mac floppy with raw
  and validated DC42 containers. Its independent test fixture reads and executes
  a generated sector. These tests do not establish this System Disk's behavior.
- BootHush's PC floppy is a 1.2 MB PC geometry, not a Macintosh GCR drive. The 286
  needs an explicit media provider; padding a Mac disk is not boot compatibility.

## Implementation order

1. Establish private input identity and add side-effect-free Mac floppy state
   diagnostics, using existing independently generated sector-loader tests.
2. Implement a checked shared logical-block interface and original providers,
   with native 68000/C16 tests, failures and bounded staging across segments.
3. Complete eligible MFS format contracts and implement volume, directory and
   fork traversal; connect Mac floppy and an explicit PC media provider.
4. Define boot-block handoff, initial memory state, ROM entry/trap services and
   System/resource loading from published interfaces and authorized observations.
5. Integrate the 286 legacy execution/service bridge and missing instructions,
   preserving native kernel/device/manager work and measured memory budgets.
6. Run the exact disk workload, fix the first evidenced failure and repeat until
   both emulator lanes satisfy the acceptance criteria; validate hardware separately.

A 68000 ROM binary cannot execute natively on a 286. The 286 lane therefore needs
an explicit boundary for disk-resident 68000 startup/System/Finder code and native
ClasSICK services. The new compatibility goal includes that execution need, but
this receipt does not silently replace the native 286 product with whole-machine
Mac emulation. Exact entry, service routing and ownership need a reviewed contract
before that bridge is implemented. Running an identical ROM binary under an
interpreter, if added as a diagnostic lane, must be labeled separately.

## Scope and remaining work

The reverse requirement (our System disk on a supported original Apple ROM)
remains required but is not the immediate selected-disk milestone. x64 stays
paused. BootHush work is prioritized by the above dependencies; Not Super Tilt Bro
continues as a supporting source of independently reviewed hardware/input evidence.
Game-derived code/assets are not automatically eligible for ClasSICK.

No selected-disk boot, physical test or compatibility success is claimed here.
Only reviewed public/original project files were used for the audits. Two bounded
Codex agents examined the emulator and 286 dependencies; private HQ, protected
media/code and held UART bodies were excluded. The lead owns the milestone and
integration decisions. Exact diagnostic verification is appended when completed.
