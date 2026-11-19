<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Original Macintosh ROM parity: scope, audit and completion criteria

2026-11-19. Owner direction SRC-0621; published-source review SRC-0622;
[ADR-0084](../adr/0084-original-rom-parity-boundary.md).
This is the current completion contract and responsibility audit, not a claim
that the contract has been implemented or that a complete API inventory exists.

After this audit, the [November 19 implementation checkpoint](system-disk-boot-20261117.md#current-checkpoint---2026-11-19)
added execution of startup instructions from the supplied System Disk on
both targets, streamed Shell segment loading and guest file I/O on generated
media. The installed 286 reached $808E and the Macintosh completed seven
in-range CPU steps. System and Finder execution remain open. The service-family table below
records the state at the time of the audit. All full-use gates remain open.

## Product definition

The Mac deliverable is an independently implemented replacement for the
original Macintosh 128K's 64 KiB ROM. It must supply the original ROM's
software-visible services and hardware startup duties so that the owner's
original disks work. It must not absorb Finder, a replacement desktop,
disk-resident packages or unrelated new features merely to make a demonstration
work. Original implementation choices and internal safety checks are permitted;
they do not justify changing the external compatibility contract.

The 286 deliverable is an installer and the standalone runtime it installs.
Together they provide the functional equivalent of those ROM services plus
execution of the disks' 68000 code. The installed runtime supplies native 286
services, PC device backends and a compatible 68k execution/service boundary.
The workload includes disk startup, System, Finder, patches, executable resources,
applications and desk accessories, not only application instructions. It has no
guest dependency on DOS, Windows, Linux or Apple ROM. Installation must be
followed by a successful cold boot of the installed system; running an installer
or installing the existing demonstration is insufficient.

This clarifies earlier application-only wording in the charter/architecture.
It preserves the native 286 design: interpreting disk-resident 68k system code
does not require translating our native 286 managers or using whole-Mac hardware
emulation as the PC OS. A Mac ROM binary cannot execute natively on a 286.

The separate independently authored System/application distribution remains a
useful project. Its completion, expanded desktop features, Linux editions and
paused x64 work are not prerequisites for completing this original-ROM component.
Their existing project-wide requirements and history are preserved separately.

## Published baseline and limits

Use published Apple interface documentation, eligible hardware specifications
and admitted black-box observations; never protected implementation as a source.
Public availability does not make ROM dumps, disassembly or confidential drafts
eligible. The source review below uses prose, interface facts and diagrams only.

| Source | Reviewed location | What it establishes |
| --- | --- | --- |
| Apple, [Inside Macintosh I, 1985](https://vintageapple.org/inside_o/pdf/Inside_Macintosh_Volume_I_1985.pdf) | I-10, Figure 1 | The ROM/RAM division and ROM service families used below. |
| Same volume | I-106, Figure 3 | System has ROM patches in its data fork and shared resources in its resource fork. |
| Apple, [published Inside Macintosh I-III](https://puyoman.net/documentation/Inside_Macintosh.pdf) | I-344; I-483 | The standard menu definition and PACK resources come from the system resource file; managers invoke them. |
| Same collection | II-383, Trap Dispatch Table Utilities | Trap interception, direct calls through saved routine addresses and RAM patches are observable interfaces. |
| Same collection | III-42, System Startup | Reset mapping, initialization/tests, disk selection and startup-block reading precede disk loading. |
| Apple, [Inside Macintosh IV, 1986](https://vintageapple.org/inside_o/pdf/Inside_Macintosh_Volume_IV_1986.pdf) | Introduction, xii, Compatibility | Volumes I-III describe the 64K ROM family, version 105/$69, used by 128K/512K machines; the 128K ROM is a later baseline. |

The baseline is the original 128K hardware/64K-ROM family, not Macintosh Plus,
SE, Color QuickDraw, HFS-era additions or later System APIs. Exact shipping ROM
revision applicability remains to be resolved from eligible evidence. The 1985
books' RAM-software descriptions do not prove what shipped on a January 1984
disk. Filename, printed part number, container length and the name "0.97" do
not establish an image's System/Finder versions.

ROM Package Manager support is required; embedding every package in ROM is not.
Likewise, ROM UI managers must load/call disk-provided definitions where the
contract requires them. Further WDEF/CDEF/default-font placement needs its own
source-level review; this audit does not infer it from a manager's name.
Original fonts, icons and sounds are not copied into our distribution. Original
substitutes must preserve required metrics/IDs/interfaces; any behavioral or
visible difference stays recorded rather than silently called exact parity.

## Comparison with the implementation

Audit baseline: `a1fa3315661f1f41ca6dc215d175c44beb007ca8`.
No new runtime tests were run for this documentation audit. Existing receipts
retain their exact source/build/test identities and limits.

- The released window ROM is a 64 KiB demonstration. Its `WINDOW_SOURCES` in
  [Build-LoadedAppROM.py](../../scripts/Build-LoadedAppROM.py) includes native
  input/window/scenes, not the memory/resource/CODE or disk-startup services.
  The separate loaded-app profile exercises an embedded original fixture.
  [The linker](../../arch/m68k/loaded-rom.ld) constrains the image to 64 KiB;
  that is not proof that all required services fit together or work.
- [Startup-file preparation](startup-files-20261118.md) opens the selected
  disk's two named resource sources but leaves product entrypoints unchanged.
  Complete fork reads, resource parsing and original sector-write persistence
  are valuable prerequisites, not System/Finder execution.
- The [banked service profile](banked-gettrap-service-20261118.md) admits only
  GetTrapAddress in its committed scope. The interpreter supports selected
  instructions and constrained execution profiles, not general 68000 startup.
- An [installer already exists](../specifications/SPEC-0110-pc286-preview-media.md):
  [preview-install.asm](../../platform/pc286/preview-install.asm),
  [preview-boot.asm](../../platform/pc286/preview-boot.asm) and
  [the media builder](../../scripts/Build-PC286PreviewMedia.py). It verifies
  an empty fixed-geometry target, writes/verifies the payload and commits its
  boot sector last. Its [qualified payload](paired-release-20261117.md) is the
  native window preview, not the complete compatibility runtime.
- Uncommitted streamed CODE, MFS overwrite and banked SetTrapAddress work is
  preserved. Its presence in the working tree is not credited as integrated,
  qualified product functionality. The recorded routine-offset uncertainty
  remains unresolved; no historical contract is changed to make a probe pass.

### ROM responsibility register

The 24 service-family labels come from I-10. Status describes the audited
current implementation, not historical software. **No row is complete.**
"Not located" means no corresponding service was found in the audited tracked
core/personality/Mac inventory; held files and historical branches were excluded.
This register must be expanded to routine-level coverage before completion.

| ROM responsibility | Existing work / present gap |
| --- | --- |
| Resource Manager | Resource-fork readers, residency and streaming foundations; full historical API, lifecycle, writes and guest integration incomplete. |
| QuickDraw | Selected rectangle/port operations and generic graphics; full ports, regions, text, pictures and callbacks incomplete. |
| Font Manager | Historical implementation not located; diagnostic fonts do not count. |
| Toolbox Event Manager | Input/FIFO foundations; historical event API and queue integration incomplete. |
| Window Manager | Generic preview window model; historical records, definition callbacks and APIs not established. |
| Control Manager | Historical implementation not located. |
| Menu Manager | Historical implementation not located. |
| TextEdit | Historical implementation not located. |
| Dialog Manager | Historical implementation not located. |
| Desk Manager | Historical implementation/accessory lifecycle not located. |
| Scrap Manager | Ownership discussions exist; functional historical service not established. |
| Toolbox Utilities | Generic dispatch/marshalling foundations, not complete utility coverage. |
| Package Manager | System PACK loading/dispatch service not located. |
| Memory Manager | Bounded manager/lifecycle foundations; expansion, master-pointer growth, purging and complete historical behavior remain incomplete. |
| Segment Loader | Preflight, image ownership and synthetic execution foundations; real startup and dynamic loading incomplete. |
| Operating System Event Manager | Input/clock foundations; historical posting, queue and low-memory contracts incomplete. |
| File Manager | Native MFS directory/fork reads; file traps, allocation, metadata mutations and full save workflows incomplete. |
| Device Manager | Block providers exist; historical device control entries, calls and completion behavior not established. |
| Disk Driver | Modeled IWM read/write probes; full driver ABI, media lifecycle and physical operation incomplete. |
| Sound Driver | Historical implementation not located. |
| ROM Serial Driver | Mac implementation not located; unrelated held UART work is not counted. |
| Vertical Retrace Manager | Interrupt/timing foundations; historical task service not established. |
| System Error Handler | Diagnostic handling exists; historical service and observable outcomes not established. |
| Operating System Utilities | Helpers/clocks/trap-address foundations; complete family coverage not established. |

Cross-cutting work must cover reset/boot, low-memory globals, traps and register/
stack conventions, direct callable targets, exception/interrupt behavior, ROM
identification and relevant patch chaining. Private-entry or ROM-relative
dependencies demonstrated by a supported disk remain compatibility gaps until
resolved; API dispatch alone is not a guarantee. These are investigation targets,
not claims that a particular protected routine has been inspected or failed.

## Completion gates

The following are engineering acceptance requirements derived from the owner's
goal. They are not quotations from Apple, nor already-passing tests.

| Gate | Required result | Current disposition |
| --- | --- | --- |
| R1 - Frozen target and interfaces | Identify the supported original ROM/hardware profile and disk versions. Enumerate all applicable routines, traps, selectors, records, globals, callbacks, errors and startup/device interfaces with edition/page or admitted observation. Mark later/RAM-only services explicitly. No unresolved applicable entry may be counted complete. | Family baseline defined; detailed inventory/version confirmation open. |
| R2 - Stock Mac image and resources | Produce a physically installable 64 KiB ROM for an unexpanded 128 KiB Macintosh. Verify chip arrangement/mapping, reproducible image, dependency closure, complete RAM/stack/heap budgets and useful application memory under combined workloads. No hidden larger-ROM/RAM or emulator-only dependency. | Demo image size proven; complete-product feasibility and hardware proof open. |
| R3 - Hardware and startup | Reset, initialize/test, provide appropriate no-disk/error behavior, select/read media and honor the historical startup handoff. Initialize required vectors/globals/services and allow disk System patches/resources to operate. Exercise keyboard, mouse, display, sound, serial, timers/VBL, internal/external disk behavior and errors within the frozen stock profile. | Partial device/probe work; integrated startup and full original hardware coverage open. |
| R4 - ROM service coverage | Every R1 ROM contract has original implementation, meaningful normal/error/boundary tests and target integration evidence. Test reentry, callbacks, direct calls and patching where required. No success-returning stubs or synthetic-only substitute for a real service. | Open for every family above. |
| R5 - Supplied-disk compatibility | Run the actual supplied disk software through the workload below on Mac and installed 286. Keep per-disk results. Never replace a missing disk operation with an embedded lookalike demonstration. | No System/Finder execution established. |
| R6 - Persistent operations | On disposable copies, create/save/reopen/rename/copy/delete files and both forks/metadata as applicable; test space/error/write-protect/eject/reinsert cases. Cold restart verifies persistence and original inputs remain unchanged. | Sector writes qualified on original fixtures; file-level acceptance open. |
| R7 - Installed native 286 equivalent | Install to a declared supported disk, verify written content, remove installer and cold boot. Run native ROM-equivalent services plus required 68000 code, patch/callback/interrupt and media semantics; execute the same R5/R6 workload. Record 286 memory/performance and device limits. | Preview installer exists; full runtime/integration and physical proof open. |
| R8 - Scope and evidence closure | Production artifacts include only this contract and necessary platform/runtime support. Retain separate demos/extensions. Publish traceable source/artifact identities and compatibility results. Repeat stock physical-Mac acceptance; distinguish physical286, emulator, component, human and provenance results. | Open; this audit closes no runtime/physical gate. |

Passing the supplied disks is **disk compatibility**, not proof of every unused
ROM service. **Mac ROM complete** requires the Mac portions of R1-R6 and R8.
**Installed 286 equivalent complete** requires its interface/service/workload
portions of R1, R3-R6 and R8, plus R7 and explicit PC resource limits.
**Paired deliverable complete** requires both; paired development remains the
priority, while their qualification and release records remain distinct.
An untested peripheral is an open gate, not "not
applicable" because the current emulator lacks it. A model-only release must be
labeled as such. Complete ROM parity does not mean universal compatibility with
every Macintosh application ever written or completion of all ClasSICK editions.

The runtime equivalent on 286 need not occupy 64 KiB or copy the Mac hardware's
internal organization; it must provide the same required guest-visible services
and enough compatible 68000 execution. The Mac executes legacy code on its
68000 directly; the 286 execution engine belongs to the PC runtime.

Before qualification, freeze numerical combined ROM/RAM/stack/heap budgets and
workload responsiveness limits for each target. Record measured high-water marks,
remaining application memory and elapsed guest times against them. Distinguish
historical reference-derived limits from explicit engineering choices; a recorded
measurement without a pass/fail threshold does not close R2 or R7. No numerical
performance threshold or full-product memory feasibility is established here.

### Supplied-disk corpus

File identity only, observed 2026-11-19; all four containers are 419284 bytes.
Only filenames, lengths and SHA-256 digests are recorded; hashes were computed
over the intact containers. No new
disk execution, executable inspection or payload export occurred. Filenames are
labels, not authenticated release identities or proof of disk contents.

| Label | Container SHA-256 | Evidence available |
| --- | --- | --- |
| Backup Disk 690-5023-A.dsk | `1ea3d868b5b0e5796d43380b81c455be40007c6fd27af3541cac25a860eca212` | Identity only; runtime/version/workload inventory pending. |
| Guided Tour 690-5002-C.dsk | `4683d74838a50592d81418da11ba702baef7a14641702c22af3bacdd58517197` | Identity only; runtime/version/workload inventory pending. |
| System Disk 690-5003-C.dsk | `1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647` | Earlier read/resource/header probes only; no startup/full-use pass. |
| Write-Paint Tour 690-5006-C.dsk | `578a58f6fd32a996d0439e537e3f72b73bca21483089ba053cfc3dbf4153dc01` | Identity only; runtime/version/workload inventory pending. |

For each disk, determine its actual boot/use sequence through admitted private
runtime observation, then freeze a checklist of every supplied application,
accessory and user workflow. Do not assume every disk boots Finder or even boots
independently: tours may have another entry and backup media another workflow.
For applicable programs exercise launch/use/exit/return, menus/dialogs, editing,
graphics/fonts, clipboard, sound, settings and persistent files. Exercise
cross-disk prompts and swapping. Printing or other peripheral workflows require
a declared provider or an explicit outstanding result; absence cannot be a pass.

Use at least three bounded cold starts of each applicable boot sequence, pinned
source/runtime/emulator/media identities and fresh disposable write copies.
Preserve failures, input hashes and per-disk outcomes. Existing System Disk
metadata observations do not count for the other images. Reference software
remains private runtime input, never our distributed payload or implementation.

## Preserve and assign existing work

Assignment is about product ownership and build inclusion, not deleting useful
code or rewriting history. Existing previews/releases remain immutable. Source
files may stay in shared directories when multiple products use them; future
build manifests must show exactly which product includes them and why.

| Existing work | Destination and use |
| --- | --- |
| Reset, Mac devices, compatible manager cores/ABI adapters | Production Mac ROM after contract, integration and footprint qualification; shared semantics also serve native 286. |
| Banked memory, 68000 interpreter and PC service bridge | Installed 286 runtime; separate Mac diagnostic builds only when useful. |
| EGA, PC keyboard/storage and preview installer/bootloader | 286 platform and installation/recovery components; extend existing work. |
| Window/product scenes and embedded loaded-app fixtures | Standalone demos/test applications and explicitly named diagnostic ROM builds. Exclude as production boot substitutes. |
| Independent future System, desktop, definition/package implementations | Separate System/application distribution and disk resources where appropriate. |
| Host viewers, captures, trace tools, original fixtures and defect controls | Developer/qualification tooling; reusable compatibility evidence. |
| Wider native APIs, x64/UEFI and Linux work | Separate port/extension tracks; preserved, with x64 still paused. |
| BootHush | Emulator/test infrastructure supporting evidenced ROM/286 gaps. Emulator implementation is not ROM functionality. |
| Not Super Tilt Bro | Game/hardware-research project; share eligible findings while retaining game code/assets in that project. |

## Immediate work order and verification of this audit

1. Expand R1 into the routine/interface ledger, prioritizing the original
   startup, System patch and disk-resource callback contracts. Resolve the
   recorded CODE offset uncertainty from numeric diagnostics and eligible facts.
2. Define separate production-ROM, installed 286-runtime and diagnostic/demo
   build manifests; keep proven component implementations and tests reusable.
3. Integrate the original hardware/boot boundary, native managers and 286 code
   bridge in paired slices, testing actual disk dependencies without skipping
   disk startup or embedding replacements for disk software.
4. Complete missing services/file mutations, all supplied-disk workloads and
   the unexercised ROM families; qualify installation and physical targets.

This audit used two read-only Codex reviews: published responsibility boundaries
and current public implementation/build/installer scope. The coordinator checked
the key source passages, build lists/linker, installer contract, receipts and
four file identities. Private HQ, held research/UART bodies and protected
implementation were excluded from worker inputs. Exact provider usage/time was
not available; no independent human provenance review is implied.

Changes are documentation and responsibility assignments only. No code was
moved/deleted, no uncommitted implementation was promoted, no runtime result was
created and no release artifact was replaced. The indexed repository guard and
required HQ documentation check are the appropriate verification for this slice.
