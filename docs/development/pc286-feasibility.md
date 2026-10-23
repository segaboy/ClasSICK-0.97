# Native 80286 PC port: feasibility and proposed roadmap

2026-10-23, SRC-0386..0389 / ADR-0077: first [C16 compile preparation](pc286-toolchain-preparation.md)
complete as a bounded tool slice:39 final compile invocations/32 objects/7 expected
rejections and2 harness refusals. P286-1 production gate remains partial:C11,
helper closure,segmented views/model/ABI/maps/fit/performance and286 execution
remain. No runtime/loader,firmware profile,target trial or release change.

Owner refinement after this study, 2026-10-23 (SRC-0383): the first working
configuration discussed is IBM PC/AT 5170, 6MHz, 640KiB conventional RAM,
IBM EGA/5154, a provisional 20MB hard disk and 1.2MB installation/recovery floppy.
The finished system installs fully to the hard disk and needs no system floppy
for regular boot/use. Real unchanged Mac128K application use, including an
appropriate Word edition to be identified, is a product goal. No single-floppy
size, memory fit, usable speed, best-selling-machine rank, exact firmware/media
profile or application compatibility is established. Broader adapter/clone
coverage below is subsequent work. This refinement does not admit a target run.
See the [native performance requirement](native-performance-plan.md).

2026-10-23;SRC-0375..0382 / ADR-0075. Lead-reviewed investigation proposal.
Public base4f231b559e491b1e4b4663d1e67ed6b8634cc056.
[Source review and dispositions](../research/pc286-source-review.md),[receipt](pc286-feasibility.json).
This study authorizes no implementation, builds, media, trials or releases. Every case below is proposed and has not been run.
It makes no claims about historical compatibility, measured fit or performance. Stable IDs identify this investigation,not an implemented hardware target or ABI.

## 1. Goal and interpretation

Owner goal: boot and run on most 286 PC-compatible machines, and launch unchanged 68k applications written for the Macintosh 128K through an original 68000 interpreter.

Interpretation used here:

- The 286 target is a **separate native 16-bit build**. It is not the x64 executable, and the 286 does not interpret the OS as a whole. The OS managers run natively on the 286. Only the guest application code and its compatibility veneers go through the interpreter.
- The existing Mac 68000 and native x64 parity and release work continues unchanged. The unified-x64 plan is a roadmap. It is not a reusable 286 binary or loader.
- The native external-application ABI/SDK gate stays closed. Guest 68k compatibility remains the way applications run.
- The target architecture requires no Apple ROM or System,nor DOS/Windows to run. Our own standalone loader/kernel would use reviewed PC firmware interfaces;no DOS/Windows startup/runtime is imported. Applications still need original compatibility ROM entry services and trap services.
- Today's guest fixture (one owned CODE0/CODE1 program, SPEC-0035) is tiny and guarded. It does not show that real applications run, that the whole ISA is covered, or that the managers are compatible.

## 2. Platform facts and constraints

| Topic | Admitted fact or inference | Consequence |
| --- | --- | --- |
| CPU | 286 is 16-bit. Protected mode has a 24-bit base (16MB physical) and 64KiB segment limits. 32-bit registers start with the 386 | No 32-bit registers. 32-bit arithmetic generally needs multiple 16-bit operations |
| Mode | Real mode keeps BIOS services available without any protected-mode return path | Baseline is real mode. No exit/reset recipe from protected mode is asserted |
| Excluded from baseline | Unreal mode, CPUID, any 386+ feature, EMS/XMS/A20, FPU | 32/64-bit math is done in software. Its cost must be measured |
| Toolchain | GCC -m16 is -m32 with .code16gcc, so it is not a 286 toolchain. OpenWatcom 1.8 (dated manual) documents small/big/huge models | One genuine 16-bit compiler must be qualified first |
| Pointers | Near pointers are 2 bytes and far pointers 4 bytes. A far increment changes the offset only. Huge pointers normalize through a runtime helper, which has a cost | A 128KiB guest RAM block cannot be one ordinary far object |
| BIOS and loader | Exact interrupt, loader and BIOS interfaces have not been admitted | Eligible source admission is required before any loader design |

Open Watcom is the leading compiler candidate. Its dated manual explicitly distinguishes WCC(16-bit) from WCC386(32-bit),and documents8086-default and286-target options;16-bit mode alone is not an ISA guarantee. The ia16 compiler family stays an unqualified alternative. No wcc,wcc386,wlink or ia16-elf-gcc was found on PATH;this is not an exhaustive installation inventory. Nothing was installed. The manual does not qualify currentv2,Windows11 hosting,C11 features,runtime behavior,licensing or imports. No third-party runtime source was read.

## 3. Architecture tradeoffs

### 3.1 Memory model

| Option | Benefit | Cost or risk |
| --- | --- | --- |
| Global huge model | Simplest port of code that does arithmetic on pointers | Normalization helper on every increment, which slows the hot paths. It does not fix the uintptr_t overlap guards or C11/library problems. It hides the segment ownership the rest of the design needs |
| Large/far everywhere | Uniform pointers | Objects still may not cross 64KiB. Spans in guest.h can wrap silently |
| **Recommended: near hot data, far code, explicit guest backend** | Interpreter state stays near and fast. Guest RAM is reached only through checked bank/page chunk accessors. Segment crossings are visible and testable | Requires redesigning the guest memory backend and a full pointer-ABI audit |

The recommended direction keeps:

- 24-bit guest addresses
- big-endian byte order
- word/long alignment faults (byte accesses may be odd)
- the SPEC-0022 transaction and no-effect-on-failure rules

This study does not choose the exact page size or API.

Guest24-bit logical addresses are software addresses,not PC physical pointers:
guest ROM at0x400000 does not demand4MiB host RAM. Map the resident128KiB guest
RAM and original compatibility bytes into lower host memory through checked
accessors. A read/long fetch can straddle two banks;preflight every chunk before
any multi-byte write. Retained views,resource-fork files,BlockMove and transaction
rollback must preserve their existing semantics. No particular page size,
far-pointer representation,ROM layout or adapter ABI is frozen.


### 3.2 The guest memory backend needs redesign

Today `cs_guest_window` returns one contiguous host view, and `cs_guest_region` assumes each region is one flat host buffer.

- Splitting the regions does not fix reads, writes or BlockMove-style copies that cross a segment boundary.
- Overlap and span checks done on numeric uintptr_t values are not valid for far segment:offset pairs. Those pairs are not in physical linear order in general.
- Callers that hold a view, such as the interpreter fetch path, the loader and the renderer, must move to chunked access. The alternative is a contract stating that a view never crosses a page.

### 3.3 A specific ABI risk (reasoned, not executed)

`guest.c` tests `(r->flags & ~7u)` and `(access & ~7u)`.

- With a 16-bit unsigned int, `~7u` is 0xFFF8.
- Promoted to 32 bits, it becomes 0x0000FFF8,so AND fails to test every bit above bit15.
- A flags value of 0x10001 (READ plus a stray high bit) would therefore pass validation.

This is not a bug on current targets. It justifies a systematic audit of literal masks,shifts,promotions,size_t/uintptr_t,int64,compiler helpers and C11 features. The masks have **not** been fully audited, and no code is changed in this study.

## 4. Native code versus guest 68k code

| Layer | Runs as | Notes |
| --- | --- | --- |
| Kernel, drivers, memory, presentation | Native 286 | Separate 16-bit build of the shared sources after the audit |
| OS/Toolbox managers (QuickDraw, events, windows, menus, text, fonts, resources, file I/O) | Native 286 | Reached through guest A-line traps |
| Compatibility ROM entry veneers, low memory, jump tables | Guest-visible data plus small veneers | Original content only. No guest ROM content or layout ABI is adopted here |
| Applications | Unchanged guest 68000 code under the interpreter | Never rewritten for the 286. No native application SDK |

Guest compatibility needs all of the following:

- the broad 68000 ISA, flags and exceptions
- trap calling conventions and callbacks
- loading of launch segments
- low-memory globals
- own-ROM entry services

Targeting the 128K does not mean every 128K application is supported. Support is declared per named eligible application profile, together with its dependencies. Applications that touch the ROM or hardware directly, or rely on private assumptions, may stay unsupported.

## 5. Coverage tiers

Each tier is assessed separately. Booting in text mode does not imply GUI or application support. No percentage of the machine fleet has been measured.

### 5.1 Boot

| Tier | Scope |
| --- | --- |
| T286-D: diagnostic boot | Legacy PC/AT BIOS, floppy, ordinary CHS services. Text output. 256KiB machines are outside the first full-GUI candidate;even diagnostic minimum remains unmeasured |
| T286-G: GUI boot | T286-D plus a bitmap adapter from section 5.2 and the full memory envelope |
| T286-A: application tier | T286-G plus storage, input and named guest application profiles |

Media candidates are 1.2MB 5.25-inch, 360KiB, 720KiB and 1.44MB. Not every BIOS supports every drive, and no sector layout or boot ABI is frozen here. The design does not require EDD/LBA, UEFI, ACPI, PCI or USB.

### 5.2 Graphics

The logical canvas stays 512x342 monochrome, unscaled and centered, until the owner decides otherwise.

| Adapter | Mode | Origin | Tier |
| --- | --- | --- | --- |
| EGA | 640x350 | 64,4 | GUI candidate |
| Hercules | 720x348 (two pages) | 104,3 | GUI candidate |
| VGA | 640x480 | 64,69 | Candidate geometry;the exact VGA hardware source/profile is not admitted here |
| CGA | 640x200 or 320x200 | none (342 rows do not fit) | Diagnostic candidate under the standing unscaled512x342 requirement;no owner policy change requested here |
| MDA | text | none | Diagnostic text only |

Both the adapter and the monitor matter. Not every monochrome card or display can show graphics. No fonts or ROM assets are copied. The presentation buffer is separate from the guest framebuffer.

### 5.3 Input

1. Keyboard first, with documented emulated pointer and click semantics.
2. Serial mouse as a later candidate. Protocol, hardware and BIOS ownership need eligible interface work. The unqualified UART code is not reused.
3. A PS/2 auxiliary mouse is not assumed in the baseline. The x64 PS/2 profile does not establish 286 coverage.

### 5.4 Storage

Use an original, documented, read-only filesystem/container that keeps Mac resource forks. The design does not assume an AT can read Mac 400K floppies, and it does not import Apple media. The current resource readers and launch pieces may be reused after the segmentation audit.

## 6. Memory envelope (design cap, not measured)

| Budget | KiB |
| --- | --- |
| Guest RAM (includes guest framebuffer, globals, heap, stack) | 128 |
| Own compatibility ROM and veneers | 64 |
| Native kernel, interpreter and manager code | 192 |
| Host working storage | 96 |
| Presentation buffer (separate from the guest framebuffer) | 32 |
| Firmware, loader, stacks and I/O buffers reserve | 64 |
| **Total** | **576** |
| Margin inside 640KiB conventional memory | 64 |

The provisional full target is 640KiB of conventional memory, with 512KiB as a stretch goal. Extended RAM is not required.

The real minimum RAM can only be set by measurement. That measurement must include the loader, BIOS data, EBDA, reserves, stacks and the code map, not just the 128KiB guest. The guest framebuffer is counted once, inside guest RAM.

## 7. Reuse and redesign

| Component | Disposition |
| --- | --- |
| `core/memory/guest.*` | Redesign. Add a banked, checked chunk backend that keeps its semantics, and audit the masks and widths |
| `core/cpu/m68k*` interpreter | Reuse its logic after the audit. Hot frame state goes near. Fetch moves to chunked access. Expand ISA coverage separately |
| `core/graphics/surface.*` | Mostly reusable. Audit `size_t`, `int32_t` and stride arithmetic. A packed512x342 mono surface is21888B(21.375KiB),below64KiB;allocation must still keep the object within its allowed segment |
| Resource parser and CODE preflight | Reuse after the segmentation audit |
| SPEC-0035 image runner | Reuse its semantics only. The 64KiB-ROM layout there is a 68000 profile, not a 286 map |
| x64 UEFI/BIOS plans and loaders | Not reusable as 286 binaries. Ideas only |
| PS/2, ACPI timing, framebuffer drivers | Redesign for BIOS-era adapters and timers |
| UART bodies | Withheld. Not considered |

## 8. Performance and verification gates (all proposed, none run)

|Proposed check|Evidence required;none executed here|
|---|---|
|Compiler/ISA|True16-bit integer/pointer ABI,286-legal own compiler listings/objects,per-segment maps,near/far calls,C11 gap and permitted-helper reports. GCC-m16/x64 tests cannot qualify this.|
|Guest memory|Cross64KiB reads/writes/fetch/copies match flat reference;odd byte permitted,odd word/long fault;preflight/no partial commit;high-bit masks reject.|
|Presentation|Unscaled logical goldens at each origin,dirty-region/cursor behavior,padding/tail canaries;adapter/monitor combinations recorded.|
|Boot/input/storage|Media/BIOS,memory,keyboard/serial/display matrix;separate diagnostic,GUI and guest-app results. No measured fleet denominator.|
|Guest apps|Own original micro-fixtures first,then separately admitted named128K-targeted third-party profiles with exact dependencies;no Apple/Microsoft binaries in this study.|
|Performance|Measure instruction-heavy,trap-heavy graphics,event/input,resource/launch and memory workloads on slow and fast286 profiles. Report guest throughput,latency and host memory;no invented full-speed claim.|
|Environment|Current Snow/VirtualBox profiles do not establish286 minimum ISA. Windows11 native hosted tests are references,not execution of a16-bit image. A suitable new environment/physical/reference trial needs separate admission/exact go-ahead.|

## 9. Sequenced first milestones

1. **P286-1: toolchain/compile-only proof.** Qualify one candidate(Open Watcom first) for Windows11 hosting,true16-bit ABI,dialect/runtime/licensing/imports;probe selected original sources,report actual widths/masks/pointers/helpers/generated ISA. No installation or probe executed here.
2. **P286-2: guest-memory contract and budget.** Choose the checked segmented backend/view lifetime,per-segment layout and temporary measured envelope;specify cross-bank commit/fault tests. A576KiB cap is not an achieved map.
3. **P286-3: portable runtime proof.** Implement only after a separate scoped assignment;keep current flat backend/68000/x64 results intact. Compare original micro-fixtures across backends;actual16-bit execution needs an admitted environment,not an ordinary Windows11 process.
4. **P286-4: diagnostic BIOS boot.** Admit exact CPU/BIOS/media/memory interfaces,then original loader/kernel and diagnostic media. Constructing images and actual boot trials are separate steps.
5. **P286-5: usable monochrome system.** Native presentation/window/input,keyboard baseline,then separately reviewed serial mouse and read-only app-transfer/storage path. EGA/Hercules candidates preserve512x342;VGA profile still needs source admission. Measure slow-machine interaction and total fit.
6. **P286-6: named Mac128K application profile.** Complete missing common68000ISA/managers/launch/resource/callback/font/file semantics,then qualify a permitted unchanged guest application. Record unsupported direct-ROM/hardware assumptions;keep performance a measured release gate.

These phase names do not replace Mac motion gaps M1-M7 or B2 criterion6.
The current paired Mac/x64 release remains the immediate milestone;the286
investigation is a separate future lane. No single executable is promised for
both286 andx64,and no third release target is silently added to acceptance.

## 10. Gates that persist

The historical, edition, full-128K, B2-criterion and native-application gates all remain. Provenance, clean-room, role and UART gates remain.

Both existing System1/ROM compatibility goals remain unchanged;this study
authorizes no Apple ROM/System/reference input or execution.

Preserve the 24 untracked originals, the held brief, old records, releases, tags and the retired wiki. No OneNote, new chats or automation.

These proposals carry no authority.
