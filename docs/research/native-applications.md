# Native application code research — 2026-10-12

2026-10-16 owner clarification, SRC-0217: the order/gate recorded below is
historical and superseded where it delays matching native PC features/workloads.
See [ADR-0043](../adr/0043-coupled-native-mac-pc-parity.md). Research proposals
are still not implementation authority; native OS parity and legacy application
compatibility are separate required lanes. Original research text follows.

Research record prepared by Claude at the owner's request. It is not a
specification and changes no code; the implementation lead decides what to adopt
and allocates identifiers. The owner asked for initial research into launching
x86 versions of System 0.97-compatible programs transparently, as if they were
68000 code, and for groundwork that carries forward, because ClasSICK will
re-create every Mac OS version through 9.2.2. The motivating example is a
clean-room program, such as a word processor, built once from its own source for
both the 68000 and x86-64.

Background: [architecture overview](../architecture/overview.md) (which already
shows native ClasSICK applications beside legacy ones),
[legacy 68000 strategy](../architecture/legacy-m68k.md),
[System 1 personality](../architecture/system1.md),
[application loading](application-loading.md),
[SPEC-0017](../specifications/SPEC-0017-trap-gateway.md) (draft),
[SPEC-0020](../specifications/SPEC-0020-shared-trap-address-services.md),
[SPEC-0021](../specifications/SPEC-0021-aline-execution.md) and
[SPEC-0022](../specifications/SPEC-0022-guest-memory-execution.md).

## In short

1. A program stays one file with one identity. Its 68000 `'CODE'` resources stay;
   a native image for the host processor travels in the same file, described by
   one small ClasSICK resource. Systems that do not know that resource (a real
   Macintosh, ClasSICK's own Mac edition) ignore it and run the 68000 code.
2. At launch ClasSICK runs the native image when one matches the machine and
   verifies, and the 68000 code otherwise. The user never chooses; the choice and
   its reason go to a diagnostic log.
3. Native code calls the same Toolbox and Operating System implementations as
   68000 code, through generated front ends, and trap patches still apply.
4. Everything that programs, desk accessories and the system can see stays in
   one app-visible memory: guest addresses, big-endian, documented layouts.
   Native code reaches it through typed accessors. Its private state can live in
   host memory.
5. Every code pointer a program can see is a guest address. A native routine
   handed to the system gets a guest address that leads to a registered native
   entry; a 68000 routine handed to native code runs in the interpreter. Calls
   nest in both directions.
6. One machine-readable interface registry (routine signatures, record layouts,
   the release that introduced each) generates the 68000 marshalling, the native
   headers and much of the testing. With instruction-set tags it carries the
   design into the PowerPC era and Mac OS 9.2.2.

## Owner decisions, 2026-10-12

After reading this record the owner decided three questions. The implementation
lead records them as sources.

- **Transparency is the requirement, in the normal system.** The owner's
  example: a 68000 version of a 1985-era word processor should run with the same
  experience as on a real Macintosh, and an x86 version of the same program
  should run too without the user knowing the difference. Native variants are
  therefore chosen in the ordinary, faithful profile, with no switch and no
  separate mode. T-1 to T-6 are requirements. Resolves NX-5.
- **Licence.** "I have no control over what others do." No interface exception
  and no licence change; ClasSICK states nothing about the licensing of programs
  others build. Resolves NX-6.
- **Order.** "I want a complete 68k experience before running any x86 code."
  No native variant is loaded or run until the owner confirms the 68000
  experience is complete. Shared foundations that 68000 programs need anyway
  (section 8, step 1) proceed now; the native-first option in step 5 is
  declined. ClasSICK's own native PC code is not affected.

## Sources and labels

- **SRC-0071**, owner's *Inside Macintosh* Volumes I–III (1985), printed pages.
  It describes ROM version 105 and the May 1984 System file; every fact still
  needs January 1984 confirmation.
- Later-era precedents, read on 2026-10-12 for design orientation only. None is
  a registered source and none supports any System 0.97 behavior. Each must be
  recorded as a source before a specification relies on it:
  - *Inside Macintosh: PowerPC System Software*, Apple, online edition dated
    3 July 1996: Mixed Mode Manager, Code Fragment Manager.
  - *Mac OS Runtime Architectures*, Apple, online edition dated 11 March 1997:
    fat binary programs, the `'cfrg'` resource, PEF structure.
  - *Carbon Porting Guide*, Apple (retired document): opaque structures and
    accessor functions.
  - Microsoft's *PE Format* and *x64 calling convention* documentation; the
    System V ABI and its AMD64 processor supplement.
  - Release history: Low End Mac, "A Brief History of the Classic Mac OS" (2012);
    TidBITS, 10 December 2001; Wikipedia, "MultiFinder".
- Labels: **[DOC85]** documented in SRC-0071 with page; **[INF]** inferred from
  documented facts; **[PREC]** later published precedent, unregistered,
  orientation only; **[HIST]** release history from secondary sources;
  **[ENG]** ClasSICK proposal; **[OPEN]** to decide (section 9).
- No Apple, Microsoft or third-party executable, ROM, source code or emulator is
  an input. How Apple's emulator, Mixed Mode Manager or loaders were implemented
  is not studied; only their published interfaces are mentioned, as precedent
  ([POLICY](../clean-room/POLICY.md)).

## Terms

| Term | Meaning here |
| --- | --- |
| Host ISA | The instruction set ClasSICK itself runs on: x86-64 for the PC edition; x86-64 or ARM64 for hosted editions |
| 68000 code | A program's `'CODE'` segments: run directly on the Mac edition, by ClasSICK's interpreter elsewhere |
| Native variant | Code for the host ISA, built from the program's own source and shipped in the same file as its 68000 code |
| Fat program | A file carrying more than one code variant (Apple's later term) |
| App-visible memory | What 68000 programs can address: heaps, handles, low-memory globals, the A5 world; SPEC-0022's guest address space |
| Guest address | A 32-bit integer naming a location in app-visible memory (24 bits significant in the System 1 profile); never a host pointer |
| Native entry | A guest address that, when called, runs a registered native routine |

## 1. What "transparent" has to mean

**[ENG]** Proposed invariants. Each becomes a test in section 7.

- **T-1 One file, one identity.** Type, creator, icon, bundle, Finder
  information and copying behave as for any program. No second file, no install
  step.
- **T-2 No choice, no prompt.** Falling back to 68000 code is silent except for
  the diagnostic log.
- **T-3 Same result.** When both variants come from one source, the screen, the
  files written and the sequence of system calls match, timing aside.
- **T-4 Other code cannot tell.** 68000 desk accessories, definition procedures
  and patches running while a native program is active see the same A5 world,
  QuickDraw globals, heap and low-memory globals as with the 68000 variant.
- **T-5 Patches still apply.** A routine replaced through SetTrapAddress runs
  when native code calls that routine.
- **T-6 Inert elsewhere.** On a real Macintosh, on ClasSICK's Mac edition, or on
  a ClasSICK build without native support, the file runs its 68000 code; the
  native parts are just data.
- **T-7 Same failure handling.** A failing native program is reported and ended
  the way a failing 68000 program is. Section 4.9 says where containment falls
  short.

## 2. What the 1984 interface already provides

**[DOC85]** Facts that shape the design:

- Resource types are any four characters. The standard types are listed, and
  any other four-character sequence may be used for types specific to an
  application (I-107–108). Several standard types hold code: `'CDEF'`,
  `'CODE'`, `'DRVR'`, `'FKEY'`, `'INIT'`, `'MDEF'`, `'PACK'`, `'PDEF'`,
  `'SERD'`, `'WDEF'` (I-107–108). Each file also has a data fork separate from
  its resource fork (I-106).
- **[INF]** A program can therefore carry a resource of a type of its own that a
  1984 system never asks for.
- Trap words reach routines through one dispatch table whose entries programs
  can read and replace (I-87–89, II-383–384). An installed address below
  `ROMBase` is taken as a patch, and replacements belong in the system heap
  (II-383–384).
- Parameter passing for stack-based routines is specified per Pascal type:
  sizes, Booleans in the high-order byte, strings and VAR parameters by address,
  records of four bytes or less by value (I-91). Register-based routines are
  reached from Pascal through interface routines, which may also store the
  result code in a global (I-95). Every routine therefore has a documented
  boundary to translate from.
- Routines passed to the system are called with the stack-based convention and
  may rely on no register except A5 (I-93, I-95). The global `CurrentA5` always
  holds the right A5 (I-95).
- The first application parameter, at A5, points to the QuickDraw globals, and
  parts of the system rely on finding A5 set this way (I-95).
- Completion routines and VBL tasks run at interrupt level, must not call the
  Memory Manager and cannot rely on unlocked handles; each has its own register
  rules (II-99, II-349). Appendix B lists the routines that may move memory and
  must not be called from an interrupt (III-211).
- A master pointer keeps flags in its high-order byte; the low three bytes are
  the block address (II-25).
- OpenDeskAcc reads a desk accessory into the application heap, and SystemTask
  gives desk accessories their periodic time (I-440, I-444).
- The Finder passes the documents to open or print as Finder information
  (II-55–58).

## 3. Precedent: how Apple later mixed two instruction sets

**[PREC]** When Apple moved to PowerPC (System 7.1.2, March 1994 **[HIST]**), it
solved the same problem in the other direction. *Runtime Architectures* and
*PowerPC System Software*:

- **Fat applications.** PowerPC code lives in the data fork and is found through
  a `'cfrg'` resource; 68000 code stays in `'CODE'` resources. A PowerPC system
  runs the fragment and ignores `'CODE'`; a 68000 system ignores `'cfrg'` and
  runs `'CODE'` (*Runtime Architectures*, chapter 7). T-1 and T-6 are
  achievable with this shape.
- **A variant table keyed by instruction set.** `'cfrg'` is an array whose
  entries identify each fragment's instruction set and point to its container
  (chapter 7).
- **Universal procedure pointers.** A 68000 procedure pointer stays an address;
  PowerPC code is reached through a routine descriptor. A descriptor begins with
  a reserved A-line word, $AAFE, which the 68000 emulator recognizes as the mode
  switch, and can hold one routine record per instruction set
  (*PowerPC System Software*, chapter 2).
- **32-bit relocations.** PEF's relocation instructions add addresses to 32-bit
  words (*Runtime Architectures*, chapter 8). PEF as published does not fit
  64-bit x86 code without extension.
- **Accessors.** Carbon later hid many system structures behind accessor
  functions (*Carbon Porting Guide*). An accessor-based native interface matches
  where the platform itself went.

What to take: the shapes (one file, an instruction-set-tagged variant table,
universal code pointers, explicit procedure signatures). What not to take:
Apple's layouts for ClasSICK's own x86 extension. There is no compatibility need
for them, and the extension has to work in a 24-bit, 1984 profile. Apple's
published formats return as compatibility requirements in the PowerPC era
(section 5), from registered sources.

## 4. Proposed design

Everything in this section is **[ENG]**.

### 4.1 Packaging

- **Variant descriptor.** One resource of a ClasSICK-chosen type, ID 0
  (type code: NX-1). It holds a magic number and format version, then one entry
  per native variant:
  - instruction set and calling-convention tag (x86-64 first; section 5);
  - minimum interface level (4.7);
  - required processor features beyond the tag's baseline;
  - where the image is: a data-fork offset and length, or a resource type and ID
    (NX-2);
  - image length and SHA-256;
  - an optional code-resource map: for a resource type and ID that holds 68000
    code (`'WDEF'`, `'CDEF'`, `'MDEF'`), the native export that replaces it on
    this instruction set.
- **68000 code stays complete** in every distributed fat program (T-6). A
  program written only for native use still carries a minimal 68000 `'CODE'`
  that shows an alert saying it needs ClasSICK on a PC, so a real Macintosh
  never crashes on it.
- **Shared resources.** `'MENU'`, `'WIND'`, `'DLOG'`, `'STR#'`, icons and the
  bundle serve both variants.
- **Thinning.** A tool can remove the native parts for 400K-disk distribution;
  it never changes the 68000 parts.

### 4.2 Choosing a variant at launch

One launch service serves every way a program starts: ClasSICK's desktop shell,
`_Launch` and `_Chain` from 68000 code, and native calls. It:

1. opens the resource fork and, if native support is enabled for the profile
   (NX-5), reads the descriptor and checks its magic and version;
2. takes the first entry whose instruction set and convention equal the host's,
   whose interface level the system provides, and whose processor features are
   present;
3. reads the image, checks length and hash, loads it and resolves every
   non-weak import (4.7);
4. runs the native variant if all of that succeeds; otherwise takes the 68000
   path ([application loading](application-loading.md), section 5) and records
   which step failed. A file with neither usable variant is reported the way
   the shell reports any program that cannot be opened.

The choice is deterministic: the same file on the same system build always gives
the same result. A per-program "use 68000 code" setting, stored by ClasSICK and
not in the program file (NX-10), serves troubleshooting and the transparency
tests.

### 4.3 Starting a native variant

The same steps as a 68000 launch, without a jump table to run:

1. Set `CurApRefNum`, `CurApName` and `CurPageOption`; reinitialize the
   application heap.
2. Lay out an A5 world in app-visible memory: QuickDraw globals below A5,
   application parameters above it, `CurrentA5` set, a jump table with no
   entries. QuickDraw, desk accessories and definition procedures rely on it
   (I-95; T-4). The program's own globals live in its image's data section,
   not below A5.
3. Reserve the application stack area as usual. It becomes the guest stack for
   68000 code that runs on the program's behalf: desk accessories, patches,
   68000 definition procedures.
4. Build the Finder information and set `AppParmHandle` as usual.
5. Load the image into host memory, give this launch its own copy of the data
   sections (D6), fill the import table, run the image's initializers and call
   its entry point on a fresh native stack.
6. Returning from the entry point or calling ExitToShell ends the program.
   ExitToShell from any native call depth abandons the native stack (an
   architecture-layer primitive) and returns to the shell; `_Chain` and
   `_Launch` likewise.

### 4.4 Calling the system from native code

- **Path.** Each routine has a generated native front end. It looks up the
  routine's current dispatch entry (SPEC-0020's directory):
  - the built-in identity: call the C implementation directly;
  - a native entry: call it directly;
  - anything else is a 68000 patch: marshal the arguments into the documented
    register or stack form and run the patch in the interpreter (T-5). A patch
    that calls the address it saved reaches the built-in routine through the
    direct-call rules (II-384).
- **Parameter classes**, recorded per parameter in the registry:
  - *value*: numbers, Booleans, points, small records, strings, and records a
    routine only reads or fills during the call (a `Rect` passed by VAR, an
    `EventRecord` to fill). Native code passes natural C types and pointers to
    host-layout copies; the front end converts.
  - *object*: anything the routine keeps or that names a system object:
    `WindowPtr`, `GrafPtr`, handles, the storage passed to NewWindow, an I/O
    parameter block (kept until an asynchronous call completes), a VBL task
    record, the `thePort` address given to InitGraf. These are guest addresses.
  - *code*: procedure pointers. Guest addresses with a signature (4.5).
- **Results and globals.** Functions return their results directly. Result-code
  globals that the Pascal interface routines set (I-95) are set too, so 68000
  and native code see the same state.
- **Packages** go through the package's dispatch entry with their selector, so a
  patched package also applies.
- **Library-only routines** (marked "Not in ROM", such as SetUpA5, II-386) are
  ordinary functions in the native interface. Native code never needs SetUpA5:
  its globals do not depend on A5.

### 4.5 Code pointers in both directions

- **Native entries.** When native code gives the system a routine (a filter or
  action procedure, VBL task, completion routine, grow-zone function, trap
  patch or native definition procedure), the interface obtains a guest address
  for it from a native-entry registry: guest address, native function,
  signature, owning program. The address is real app-visible memory (a small
  block in the application heap, or in the system heap for a patch, per II-384)
  holding a marker the interpreter recognizes (NX-3). On reaching the marker,
  the interpreter looks up the address, not the bytes; an unregistered address
  stops deterministically, so copying or forging a marker yields nothing.
  Entries end with their owning program.
- **Calling any guest code address** from native or system code uses one
  service, *call(address, signature, arguments)*: a native entry runs directly,
  a built-in identity runs directly, anything else runs in the interpreter until
  it returns to a sentinel address. The system needs this service anyway to call
  68000 definition procedures and desk accessories for 68000 programs.
- **Nesting.** Native program → system → 68000 window definition → trap →
  system → native filter, and so on. Each guest call is a nested interpreter run
  with its own instruction budget; guest code uses the application stack, native
  code the host stack. The interpreter must be re-entrant; the guest stack
  pointer and A5 must be exactly restored on every return (I-95); depth is
  bounded and overflow stops deterministically.
- **Signatures** cover both documented conventions: stack-based (I-91, I-93) and
  register-based, including the interrupt-level register rules (II-99, II-349).
- **Code resources.** Before calling a code resource from a fat program's file
  (its `'WDEF'`, say), the system checks that program's code-resource map
  (4.1) and calls the native export instead when one is listed for the host.

### 4.6 Memory

- **One app-visible memory.** System records stay in their documented layouts
  and byte order. Native code reads and writes their fields through generated
  accessors, from the same registry that tells the system's own C code how to
  parse and serialize guest records ([System 1 personality](../architecture/system1.md)).
- **Byte spans** such as text and bitmaps can be reached through a host window
  (`cs_guest_window` exists) with no byte swapping. A host pointer derived from a
  guest address is valid only until the next call that may move or purge memory
  (III-211) or the next callback, the same rule as for a dereferenced handle.
  Debug builds enforce it with a heap-movement generation count.
- **Private native memory.** A native program may keep private data in host
  memory: its image's data sections or a ClasSICK native allocation service.
  Anything the system or other code must see goes in app-visible memory.
- **Addresses.** Guest addresses are 32-bit integers in every era. In the
  System 1 profile 24 bits are significant and master-pointer flags occupy the
  high byte (II-25). Native code never stores flags in address bits; it uses the
  Memory Manager calls.
- **Size.** The app-visible memory available follows the profile and applies to
  both variants. Private host memory is extra, which can make a native variant
  behave differently near memory limits (NX-7).

### 4.7 The native image and its toolchain

- **Converted at build time.** A ClasSICK tool converts a standard linker output
  into a ClasSICK native image; the system's loader understands only that image,
  so it stays small and is fully specified by our own specification. The image
  holds: a header (magic, version, instruction-set and convention tag, interface
  level, processor features); loadable sections with read, write and execute
  permissions; relocations that only add the load address, at 32- and 64-bit
  widths; an import table; an export table (entry point, initializers, named
  exports for code resources and callbacks). No thread-local storage or
  host-OS features. The converter rejects anything the loader does not support.
- **Imports by name.** Each import is a library and a routine name, optionally
  weak. The loader fills a table; a missing weak import reads as absent, so one
  image runs on later, larger interfaces and can test for optional routines.
- **Interface levels.** Each era's release exposes exactly that era's routines,
  selected by the registry's era tags. An image's minimum level gates launch.
  Level 1 is the System 0.97 set, which depends on SPEC-0017 Q7 (which traps
  the January 1984 system implements).
- **No fixed addresses.** Native code reaches the system only through its import
  table, and app-visible memory only through addresses the system gives it.
  This is the seam for later isolation (4.9).
- **One source, two builds.** The 68000 build (GCC for m68k, then a tool that
  writes `'CODE'` segments and the jump table) and the native build come from
  the same source; one packager writes the fat file. The project already builds
  68000 code through ELF objects for the Mac ROM work.

### 4.8 Calling convention per instruction set

Fixed once per instruction set for every ClasSICK edition on it (D7), so one
program file runs on the PC edition and on hosted x86-64 editions alike. For
x86-64 there are two candidates (NX-4):

- **Microsoft x64** (PE/COFF output). The PC edition's pinned LLVM-MinGW
  toolchain and the hosted Windows edition already use it. It has no red zone
  below the stack pointer. A hosted Linux edition would declare its boundary
  functions with the compilers' Microsoft-convention attribute.
- **System V AMD64** (ELF output). The default for Linux and ELF toolchains. It
  lets functions use 128 bytes below the stack pointer (the red zone). The PC
  kernel runs at CPL0 (SPEC-0008) and is built with `-mno-red-zone`; once
  interrupts are enabled, an interrupt taken at CPL0 on the same stack would
  overwrite a program's red zone. Programs would have to be built without it,
  or every interrupt would need its own stack (IST).

Lean: Microsoft x64 at the x86-64 boundary, since it costs nothing on the first
two editions and has no red-zone hazard. Decide by ADR; ARM64 later.

The PC kernel is also built with `-mgeneral-regs-only`, while native programs
will use SSE. Before running native code the PC edition must have SSE enabled,
and must keep vector state intact across anything it runs in between (NX-9).

### 4.9 Faults, exit and trust

- **Faults.** A processor exception in native program code ends the program the
  way a failing 68000 program ends: the system error presentation, then the
  shell. On the PC this builds on the owned exception handling of
  [SPEC-0008](../specifications/SPEC-0008-x64-exceptions.md) and
  [SPEC-0019](../specifications/SPEC-0019-native-fault-checkpoint.md).
- **Trust.** Native code runs with the system's privileges: CPL0 on the PC
  edition, inside the ClasSICK process on hosted editions. A native variant is
  therefore less contained than the same program's 68000 code, which receives
  no host pointers ([legacy 68000 strategy](../architecture/legacy-m68k.md)).
  Installing a native program is as trusted as installing system software. A
  wild write can corrupt the system, so recovery after a native fault is best
  effort, not guaranteed.
- **Later isolation.** Because native code reaches the system only through its
  import table (4.7), a later edition can move native programs to user mode with
  paging, or to a separate process, without rebuilding them. That step needs
  its own ADR. The image hash detects corruption, not tampering; signing is a
  later option.

### 4.10 Profile and licence questions for the owner

The owner decided both questions on 2026-10-12 (Owner decisions, above).

- **Profile.** The System 1 personality requires an extension contract for every
  difference from `system1-1984`. The native interface is a
  `classick-extended` contract. Proposal: allow native variants under
  `system1-1984` too, provided T-4 holds and 68000 code observes nothing
  different (NX-5).
- **Licence.** ClasSICK is GPL-3.0-or-later. Whether a separately distributed
  program that uses the ClasSICK native interface must also be GPL is a legal
  question. Projects in a similar position publish an explicit exception for
  interface headers and import stubs. Any change needs the owner's explicit
  approval ([licensing](../development/licensing.md)). I am not a lawyer and
  this is not legal advice. 68000 programs link to nothing of ours, so the
  question arises only for native variants (NX-6).
- **Re-creating someone else's program.** A clean-room re-creation of a
  third-party program, such as the word-processor example, is its own clearance
  project: an original name (trademarks), none of the original's code, and the
  file-format and interface questions noted in the
  [asset replacement research](asset-replacement.md). The mechanism in this
  record does not change any of that. Translating existing third-party 68000
  executables into native images ahead of time is a different matter again: it
  makes a derived copy of someone else's code, and is not proposed. The
  legitimate speed path for such programs is the interpreter, and later a JIT
  under its own ADR (legacy 68000 strategy, stage 6).

## 5. Groundwork that carries to Mac OS 9.2.2

**[HIST]** What each era adds to the boundary. Dates are from the secondary
sources above; register primary sources before relying on them.

| Era | What programs ship | App-visible addresses | What the boundary has to add |
| --- | --- | --- | --- |
| System 1 (January 1984) onward | 68000 `'CODE'` | 24-bit | 68000 ↔ native (this record) |
| System Software 5 with MultiFinder (1987) | Same; several programs open at once | 24-bit | One A5 world, heap and native image instance per program |
| 68020–68040 processors, floating-point unit | Wider instruction set; line-1111 words carry FPU instructions | 24- or 32-bit | Interpreter processor profiles; line-1111 cannot serve as a ClasSICK marker |
| System 7.0 (1991) | Same | 32-bit addressing available; programs must keep address high bytes clean | Address width per profile |
| System 7.1.2 (March 1994) | PowerPC fragments in the data fork via `'cfrg'`; fat programs; 68000 code under emulation | 32-bit only on PowerPC | PowerPC as a third code kind; `'cfrg'`/PEF loader; routine descriptors; import libraries |
| Later System 7 | CFM-68K: 68000 code fragments, also through `'cfrg'` | 32-bit | The same loader for 68000 fragments |
| Mac OS 7.6 (January 1997) | Same | 32-bit only | 24-bit mode retired |
| Mac OS 8.5 (October 1998) | PowerPC required for the system; 68000 programs still run under emulation | 32-bit | None new; the 68000 path stays |
| Mac OS 8 and 9 with CarbonLib | Carbon programs: many structures opaque, reached through accessors | 32-bit | Accessor style already matches the native interface |
| Mac OS 9.2.2 (December 2001) | Last classic release | 32-bit | End of the roadmap |

**[ENG]** Durable decisions to record as ADRs now, because every later era leans
on them:

- **D1 One app-visible memory model.** 32-bit big-endian guest addresses, 24
  bits significant in early profiles, the same for 68000, PowerPC and native
  code.
- **D2 One implementation per routine.** Every front end (68000 trap, PowerPC
  import library, native import) is generated from one interface registry that
  records signatures, record layouts and era tags.
- **D3 Universal code pointers.** Every program-visible callable is a guest
  address; native entries are registry-checked; mode switching is keyed by an
  instruction-set tag and works between any two of 68000, PowerPC and native,
  mirroring the published routine-descriptor idea.
- **D4 Variant selection is its own loader stage**, above the format loaders
  (`'CODE'`, `'cfrg'`/PEF, ClasSICK native image), with a preference order fixed
  per release (native, then PowerPC, then 68000) and diagnostics.
- **D5 Native code reaches the system only through imports** filled by the
  loader: by name, weak when wanted, gated by interface level.
- **D6 Per-launch native instances from the start.** No mutable state shared
  between launches, ready for several programs at once.
- **D7 One calling convention per instruction set** across all editions.

**[ENG]** An instruction-set tag registry, also by ADR. Proposed initial rows:

| Tag | Code | Calling convention | Container | Era first needed |
| --- | --- | --- | --- | --- |
| 68000 | 68000 (later 68020–68040 profiles) | Documented Pascal stack and register conventions | `'CODE'` segments | System 1 |
| x86-64 | x86-64 baseline | Microsoft x64 or System V (NX-4) | ClasSICK native image | This extension |
| ARM64 | ARMv8-A baseline | To decide | ClasSICK native image | Hosted ARM64 editions |
| PowerPC | 32-bit PowerPC | Published PowerPC runtime conventions | `'cfrg'` + PEF | System 7.1.2 profile |

## 6. What this asks of the current code

| Area | Today | Needed |
| --- | --- | --- |
| Dispatch directory (SPEC-0020) | 512 logical addresses; Get/Set and unsupported tokens; direct calls | Entries typed as built-in identity, guest code or native entry; a lookup per native call |
| Interpreter (SPEC-0021, SPEC-0022) | Bounded steps; MOVE, branches, BSR/RTS and A-line; no JSR/JMP | JSR/JMP, a call-into-guest service with a sentinel return, re-entrant nested runs with per-call budgets |
| Guest memory (SPEC-0022) | Up to eight checked regions; `cs_guest_window` | Host windows tied to a heap-movement generation; heap-backed native entries |
| Memory Manager | Phase-1 research drafts | The same routines' native front ends; result globals kept consistent |
| Resource and File Managers | Research only | Read the descriptor; read data-fork ranges into host memory |
| Segment Loader | [Application loading](application-loading.md) research | Launch service with variant selection; native launch; ExitToShell unwinding |
| PC architecture layer | CPL0 owned exceptions; kernel without red zone or SSE | Program stacks and an abandon-stack primitive; native-fault path; SSE enabled; IST if System V is chosen |
| Tools | 68000 GCC pipeline for the Mac ROM | Image converter, fat-program packager, descriptor writer, generated native headers |

## 7. Test plan

All tests use an original fat test program generated by a project tool: one
original C source built as 68000 `'CODE'` and as an x86-64 native image, with
shared resources. It opens a window, draws, runs a modal dialog with a filter
procedure, uses a custom window definition procedure from its own resources,
allocates and resizes handles, and writes a file. No Apple or third-party program
is used.

| ID | Test |
| --- | --- |
| NT-01 | Selection: descriptor missing, unknown version, wrong instruction set, interface level too high, missing processor feature, hash mismatch, missing non-weak import. Each runs the 68000 code and logs the failing step; with no 68000 code, the shell's normal error |
| NT-02 | Transparency (T-3): one scripted input sequence run forced-68000 and native; compare screen hashes, written files and the system-call log |
| NT-03 | Observer (T-4): a 68000 test desk accessory records `CurrentA5`, `thePort`, the heap zone and chosen low-memory globals; identical under both variants |
| NT-04 | Patch (T-5): a 68000 patch on one routine; a native call runs the patch once and the original once |
| NT-05 | Callbacks in both directions and both conventions (stack-based filter procedure; register-based completion routine) |
| NT-06 | Nesting depth: native → 68000 → native, repeated; guest stack pointer, A5 and host stack balanced; overflow stops deterministically |
| NT-07 | A copied or forged native-entry marker at an unregistered address stops deterministically |
| NT-08 | Debug build detects a host pointer used after a call that may move memory |
| NT-09 | Debug build stops a native VBL task that calls a routine from Appendix B (III-211) |
| NT-10 | ExitToShell from native depth; a native fault reaches the system error path and the shell |
| NT-11 | Inert elsewhere (T-6): the same file runs its 68000 code on the Mac edition and on a build without native support. Emulator runs only with the owner's go-ahead; later real Mac OS releases only by owner observation |
| NT-12 | Weak import absent on a smaller interface level; the program takes its fallback path |

## 8. Suggested order

Per the owner's decision, steps 3 to 6 wait until the owner confirms the 68000
experience is complete.

1. Pieces 68000 programs need anyway: the interface registry format (start with
   the Memory Manager phase-1 routines and the two trap-address services),
   JSR/JMP and the call-into-guest service, typed dispatch entries.
2. Paper decisions: ADRs for D1–D7 and the tag registry; NX-1, NX-3, NX-4.
3. Specifications for the native image, the descriptor and the native-entry
   registry; then the converter and packager.
4. The launch service with variant selection.
5. A first native program on the PC edition that opens a window and draws. It
   needs the Toolbox managers but not the interpreter, so it could run before
   the 68000 application runtime is complete. The owner declined native-first.
6. The transparency tests as soon as both paths run the test program.

## 9. Open questions

| ID | Question | Resolution path |
| --- | --- | --- |
| NX-1 | Descriptor resource type code: must avoid every Apple-defined type through Mac OS 9.2.2 and common developer types | Registered later-era type lists; ADR |
| NX-2 | Image in the data fork by default (Apple's precedent; keeps it out of the guest heap) or in resources (for programs that use their data fork) | Specification; support both through the descriptor's location field |
| NX-3 | Native-entry marker: a ClasSICK-private A-line word that no later Apple release assigns; the published routine-descriptor shape and $AAFE marker, so the PowerPC-era mechanism is the same one; or no marker and a reserved address range (native entries would then not be heap blocks, unlike the patch placement II-383–384 describes) | Register *PowerPC System Software*; later trap lists; ADR. Lean: routine-descriptor shape |
| NX-4 | x86-64 boundary convention (Microsoft x64 or System V) and converter input (PE or ELF) | ADR; check the pinned toolchain's support for the chosen convention attribute |
| NX-5 | Native variants under `system1-1984`, or only under `classick-extended` | Decided 2026-10-12: chosen in the normal profile, transparently |
| NX-6 | Licence exception for the native interface headers and import stubs | Decided 2026-10-12: no exception, no licence change |
| NX-7 | App-visible memory size per profile, and whether private host memory should count against it for parity | Profile specifications |
| NX-8 | Where interrupt-level native callbacks run on the PC (VBL timing, completion routines) and how the Appendix B rule is enforced | Event Manager, Vertical Retrace and Device Manager specifications |
| NX-9 | SSE enablement after firmware handoff; vector-state handling if the system ever uses those registers | PC architecture specification; black-box observation on cleared platforms only |
| NX-10 | Where the per-program "use 68000 code" setting lives without modifying the program file | Shell and settings design |
| NX-11 | Native desk accessories (`'DRVR'`) and other system-level code resources | Later extension; out of scope for the first version |
| NX-12 | Keeping two builds of one source behaviorally identical: fixed-width types, file serialization, and numerics (host floating point can differ from the system's numerics routines; a program that needs identical results uses ClasSICK's routines from both variants) | Native interface guidance; NT-02 |
| NX-13 | Whether later real Mac OS releases ignore the descriptor and data-fork image as expected | Owner observation in those eras |

## Not in this record

- Any routine's semantics, which belong to each manager's specification.
- PowerPC execution, `'cfrg'`, PEF and routine-descriptor layouts, beyond noting
  them as later compatibility requirements.
- Running existing third-party programs natively, which this record does not
  propose (4.10).

Claude (Anthropic; session configured as `claude-opus-5-5`) prepared this record;
human provenance review pending.
