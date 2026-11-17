<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Owner-selected System Disk boot milestone - 2026-11-17

## Direction and exact target

SRC-0612 records the owner's new priority: make the selected Macintosh System
Disk boot against ClasSICK on both the original Macintosh and the 286. This
follows the published paired interactive-window preview; published releases and
their qualification remain immutable.

The owner further clarifies that executing and fully using this boot disk is the
end-to-end target for a complete Mac ROM and a standalone 286 system capable of
executing 68k code. Reaching the first desktop frame is only an intermediate
checkpoint. Disk-resident startup, System, Finder, applications and accessories
are part of the intended 286 compatibility workload, not just an embedded demo.

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
| Mac128K in BootHush | Cold start with independently built ClasSICK ROM and the exact selected disk reaches the disk's usable desktop and completes the full-use workload below. | No selected-disk run; guest floppy/System boot path absent. |
| Native 286 in BootHush | Native 286 ClasSICK provides the required ROM/OS services and a defined legacy-code/media bridge for the same disk payload; the same full-use workload completes without a host OS dependency inside the guest. | No selected-disk run; bridge, filesystem and broad 68000 execution absent. |
| Original physical Macintosh | The independently built replacement ROM and selected disk repeat the Mac acceptance workload on identified original hardware. | Required, unverified; emulator success does not close this row. |
| Physical 286 | An identified native 286 machine repeats the selected workload using a declared media-transfer method. | Unverified; no claim from emulator behavior. |

The desktop must come from successful disk-system startup, not the built-in
ClasSICK scene or a substituted original fixture. Log each stage reached and the
first unresolved dependency; a mounted image, motor command, successful sector
read, screenshot or absence of an immediate fault is not a boot pass.

For each executable lane, record three bounded cold starts with pinned source,
ROM/native image, disk and BootHush identities; retain failures and repeatability.
Verify that the original input hash is unchanged. Start with read-only boot
triage; use an explicit private disposable copy/overlay for write tests and
verify persistence from a newly started guest. Keep original-fixture no-disk,
truncated-media and injected-read-failure controls separate from reference runs.
Capture useful private frames/video when a visual startup milestone is reached;
reference pixels do not enter the public evidence gallery automatically.

## Full-use workload

Inventory the exact disk's available user-facing functions after admission and
track each as passed, failed, blocked or not present. Acceptance includes:

- Complete startup into Finder and sustained usable keyboard/mouse interaction.
- Browse the volume and open files; launch, use and close every application and
  desk accessory actually supplied on the selected disk, returning to Finder.
- Exercise their menus, dialogs, text entry, rendering and applicable clipboard,
  font, sound and settings functions using an explicit repeatable checklist.
- Create/save, reopen, rename, copy and delete original test files on disposable
  writable media; verify the saved state after a cold restart and retain the
  pristine input's unchanged identity. Eject/reinsert and storage-error handling
  are tracked where applicable.
- Match the substantive workload on Mac and native 286 and record unsupported
  operations explicitly. Peripheral-dependent operations need an actual provider
  or a named outstanding limitation; they are not silently counted as passed.

A full pass establishes compatibility with this exact disk and workload. ROM
services, CPU instructions and exception behavior that the disk does not exercise
still need independent coverage before an unrestricted complete-ROM or complete-
68000-runtime claim. Keep an interface/instruction coverage inventory alongside
the end-to-end result; successful startup alone is insufficient.

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
6. Add private writable-media/overlay support where needed for the full-use
   workload, preserving the original disk, and verify save/restart persistence.
7. Run the exact disk workload, fix the first evidenced failure and repeat until
   both emulator lanes satisfy the acceptance criteria; validate hardware separately.

A 68000 ROM binary cannot execute natively on a 286. The 286 lane therefore needs
an explicit boundary for disk-resident 68000 startup/System/Finder code and native
ClasSICK services. The new compatibility goal includes that execution need, but
this receipt does not silently replace the native 286 product with whole-machine
Mac emulation. Exact entry, service routing and ownership need a reviewed contract
before that bridge is implemented. The owner's full-use clarification includes
execution of disk-resident 68k system code within this compatibility boundary.
Running an identical ROM binary under an
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

## First BootHush diagnostic increment

BootHush 0e9579224364257e6b2c32263561bf07ecaad573 adds the opt-in
`--mac-floppy-state 1` snapshot: modeled media, motor, track/controller state
and six device-activity counters, with no media bytes and no guest side effects.
Its existing third-party device/CPU components remain emulator dependencies, not
ClasSICK implementation inputs. No emulator implementation was imported here.

Native Windows active-profile twins match SHA-256
`ec4f037699b1e116ca74775f5b257e5deafd3dcfdfc973342ae6bfaa16832730`.
Eleven focused floppy/register tests passed, plus 22 shared CLI regression tests;
two optional ClasSICK-fixture tests were explicitly unset and skipped. Original
track-64 loader observations with the flag on/off match PC, cycles, registers,
loaded bytes and frame. Ignored writes preserve source media. The builder retains
an exactly pinned existing x64 archive; this is not a fresh x64 source build.
[BootHush interface and limitations](https://github.com/segaboy/boothush/blob/0e9579224364257e6b2c32263561bf07ecaad573/docs/mac-floppy.md).

### Released-ROM baseline

The published ROM `5d48f7877386e30f8e45e0456f528e93c1990aa7dd5b7ef37b40ddd7cf1de5a0`
was observed for 100,000,000 requested cycles in two headless runs: no disk, and
an independently generated all-zero 409,600-byte raw fixture. This fixture is
not a filesystem or historical disk. Both runs reach the cycle limit with equal
PC/cycles/frame. The inserted fixture is reported present and write-protected,
but the motor and controller remain off and all six activity counters are zero.
The fixture hash is unchanged. This corroborates that this preview has no disk
startup attempt; it is not a selected-System-Disk boot result.

Local command/result records, exact fixture identity and frames are retained in
`system-disk-boot-20261117/baseline` under the task-output directory. Reproduce
with the above pinned executable/ROM, `--machine mac128k --cycles 100000000`,
`--timeout-ms 20000 --mac-floppy-state 1 --mac-cpu-state 1 --frame OUTPUT.pbm`,
first without media and then with `--floppy` pointing to a newly generated
400 KiB zero fixture. These captures show the existing own-product preview and
add no new user-visible startup milestone. No reference image was used.

## First storage foundation

SPEC-0111 / IMPL-0141 / TEST-0200 now provide an original read-only logical-block
service with checked ranges/capacities and staged publication after every read
succeeds. Late provider failure leaves caller output unchanged. Hosted Release
and sanitizer checks, routine CTest integration, native 68000 and C16 compilation,
and original RAM-provider diagnostics on both BootHush targets pass. Deliberately
broken early-publication and missing-range implementations fail on both targets.
[Exact implementation, observations and budgets](block-read-foundation-20261117.md).
This is storage infrastructure, not a floppy driver, MFS or System Disk boot.

## Parallel port progress

Not Super Tilt Bro 3c751064f8b1244c355a13fa3631ecde6509dfd8 adds one aerial jump
for each player in its opt-in native-jab title. Twenty new scenarios, four mutation
controls and the prior 34-scenario suite pass using the same current BootHush
executable; six legacy profiles remain byte-identical. Original-art PNG/GIF
evidence is retained by the port project.
[Port evidence and limits](https://github.com/segaboy/Not-Super-Tilt-Bro/blob/3c751064f8b1244c355a13fa3631ecde6509dfd8/docs/aerial-jump-evidence.md).
The cross-project contribution is additional scheduled-input/state/framebuffer
validation of BootHush. Game-derived code, tests and assets stay in that project
and do not implement ClasSICK services. No new physical-hardware claim follows.

## MFS metadata and native disk transport

Later on 2026-11-17, SPEC-0112 / IMPL-0142 / TEST-0201 add the bounded MFS
volume-header and single-directory-entry reader. Host Release/sanitizer checks,
routine CTest and seven native Mac/286 baseline/defect outcomes pass.
[Metadata receipt and remaining format gates](mfs-metadata-20261117.md).

SPEC-0113 / IMPL-0143 / TEST-0202 add an explicit native 286 provider for a
declared subrange of original PC floppy media. Six actual-media native tests
pass, including MFS header/known-entry acquisition and injected-read failure.
[Transport mapping, identities and limitations](pc286-floppy-provider-20261117.md).
The Mac lane still needs its physical floppy provider; neither lane mounts or
boots the selected System Disk. Directory enumeration/map/fork services and
the native 286 legacy execution boundary remain outstanding.

BootHush 5d613614dd4f17d605de75ecc3f38c9129079488 adds a selected-sector GCR
checksum fault. Sixteen focused and 22 default CLI tests pass; two optional
ClasSICK-fixture tests were unset and skipped. Original guest retries reject
the persistent fault, and a fresh run with injection removed succeeds without
changing the input image. This is a boot-error testing capability, not a boot.
[Documented fault interface](https://github.com/segaboy/boothush/blob/5d613614dd4f17d605de75ecc3f38c9129079488/docs/mac-floppy.md).

Not Super Tilt Bro 67b58989bf3ca78206456bcaadd27405ce622940 adds opt-in
native aerial steering/drift for both players. The focused 14 gameplay cases,
522-step arithmetic probe, six native-call checks and defect controls pass on
the same 7adb8a74 BootHush build; broader 20/34-case regressions retain their
previous exact emulator identity. Seven older profiles remain byte-identical.
[Original gameplay evidence](https://github.com/segaboy/Not-Super-Tilt-Bro/blob/67b58989bf3ca78206456bcaadd27405ce622940/docs/aerial-control-evidence.md).
This supplies additional native-call/input/observer validation; game code and
assets remain in the port and do not become ClasSICK implementation inputs.
