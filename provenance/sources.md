# Source catalog

Access/review date for external sources: **2026-10-07** unless noted. Catalog
eligibility applies to the stated scope, not every resource reachable from a URL.
No Apple executable, source, disassembly, decompilation, or copied asset was used.
Project-owned black-box native trials are recorded separately below.

## SRC-0100 - Own checked memory and compiler verification

2026-10-12, SPEC-0022 / TEST-0030, compiled source
`61c6c8de0963ee90748d284dab26b19727563e26`. Original twelve-step data/control
fixture, 1,110 hosted checks in nine configurations, thirty additional standalone
width/endian audits and ten 68000 object audits/two complete relocatable links.
The independent assembler agrees with the 36-byte fixture. CI38292360762 passes
both jobs; combined objects, fixture, source fingerprints and function frame
values match local after ordering the frame list. Existing six EFI/two ROM
profiles and ROM input fingerprints are unchanged and match CI. No new probe
boot linkage, VM/emulator launch, operator input or historical allocation claim.
[Report](../docs/development/guest-memory-evidence.md) retains scope and safe
checksums, no private paths/IDs/binaries. No Apple/Microsoft implementation,
emulator source or firmware internals; UART bodies untouched. Human review pending.

## SRC-0099 - Owner's application transparency and 68k-first gate

Owner decisions of 2026-10-12, carried by Claude's native-application handoff
and explicitly confirmed in the implementation chat. The owner requested
research into transparent x86 versions of compatible programs and groundwork
for clean-room versions through Mac OS 9.2.2. His exact decisions:

The initial request, verbatim:

> Next, there is an opportunity to run x86 code in this operating system. For
> instance, if we were to clean-room Microsoft Word or something... we could
> make an x86 version. We need to do the initial research into how we can launch
> x86 versions of compatibile system0.97 software, transparent to the user, as
> if it were 68k code. We also need to lay the groundwork for this to move going
> forward. I say that, because we will do a clean room of every version of Mac
> OS until 9.2.2.

The subsequent decisions, verbatim:

> I don't understand what strict 1984 mode is. This is what I want - if I
> have a 68k version of Word 1.0, I should be able to run it with the same
> experience as a real Mac. If I run an x86 version of Word 1.0 (which doesn't
> exist except for the sake of argument), I should be able to run it, too, and
> not know the difference as a user. It should be transparent.

> License: I have no control over what others do.

> I want a complete 68k experience before running any x86 code.

> Write the handoff.

The owner now explicitly confirms the gate includes building, loading and
running x86 application code, native application tests and the native image
loader; own native OS/core/tools are unaffected. Only the owner can open it.
T-1 through T-6 in the [research record](../docs/research/native-applications.md)
are future transparency requirements in the ordinary experience, with no
separate user mode. GPL-3.0-or-later and inbound terms remain unchanged; no
interface exception or declaration about others' licensing is added.
The word-processor example is hypothetical for the x86 variant; possession,
January1984 support and rights to third-party application bytes are not inferred.

Allowed now: a routine/record interface registry, JSR/JMP and bounded nested
guest calls, built-in/guest typed dispatch and a separate CODE-only selection
stage. [Runtime plan](../docs/development/m68k-runtime-plan.md), ADR-0026.
The owner requires a go-ahead before any future emulator/VM run. Published
sources only; no Apple/Microsoft implementation, emulator source, delegation,
new chats, automation or OneNote. Research snapshots 37534d7 (public) and
3e0d917 (HQ) were fast-forwarded exactly; no local draft was overwritten.
Requirement source, not historical/compatibility/build evidence. Later-era
orientation manuals in the research are unregistered and supply no specification
facts until independently reviewed/registered. Human provenance review pending.

## SRC-0096 - Owner continuation alongside allocation research

2026-10-12: "OK, Claude is researching. Now, get to work." Continue the
checked guest memory/instruction work while Claude researches Memory Manager
contracts/test plans. Ordinary original specifications/code/tests/evidence and
Git/HQ/wiki publication are authorized. No changes to Claude's draft files,
UART body role, Apple input exclusions, platform admissions or edition gates.

## SRC-0097 - Published Motorola data and control instruction interfaces

Motorola, 1992 M68000 Family Programmer's Reference Manual, published ISA
prose and instruction-format tables: 1.7.1/2 (1-25/27), 2.2.4/2.2.5 (2-6/7),
2.2.17/18 (2-18/19), 2.6.1 (2-28),
Table 3-19 (3-19), MOVE (4-116/118), MOVEA (4-119/120), Bcc (4-25/26),
BRA (4-55/56), BSR (4-59/60). Existing SRC-0093 supplies MOVEQ/NOP/RTS.
[Official successor PDF](https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf).
Access 2026-10-12 via web parsing, no local PDF/hash or visual-review claim.
68000-only interpretation distinguishes later long branch displacement forms.
Eligible ISA facts, not January 1984 OS behavior. No examples/equates/sample
code, emulator source or external implementation incorporated.

## SRC-0098 - Original checked memory contract and fixtures

SPEC-0022 / ADR-0025 / IMPL-0026 / TEST-0030. Independently authored map,
temporary CPU frame/deferred store, 36-byte data/control program, condition
truth masks and boundary fixtures. Codex-assisted lead self-review; human
provenance review pending. Claude allocation drafts remain separate research.

## SRC-0095 - Own A-line instruction and native return observations

2026-10-12, SPEC-0021 / TEST-0029, compiled source
`f29113b13775102f294d76e9c3deb1691228b1c8`. Three fresh PC starts interpret
the original instruction fixture; two fresh Snow 128K starts execute actual
vector10/RTE and three native service calls, register/SR/return checks and scene.
[Report](../docs/development/aline-execution-evidence.md) separates these CPU
paths and unimplemented historical/legacy/complete CPU boundaries. Retain only
own post-success selected PC registers and 64 own Mac diagnostic RAM bytes.
No firmware internals, emulator implementation, Apple ROM/code or assets used.
No new operator evidence. VM off/unchanged VDI, both Snow windows closed normally;
process exit codes not retained. CI38284131256 passes both jobs, six EFI/two ROM
profiles and all ROM source fingerprints match local. Human review pending.

## SRC-0092 - Owner continuation to original A-line execution

2026-10-12: "continue" after shared services passed and the next instruction-call
boundary was proposed. Original bounded CPU/service contracts, fixtures,
implementation/tests/builds and ordinary Git/HQ/wiki planning. Necessary checks
on the same cleared platforms retain fresh isolation/prior media; no new emulator,
Apple input, UART body work, delegation, OneNote or physical writing.

## SRC-0093 - Published Motorola instruction and exception interfaces

Motorola MC68000UM ninth edition 1993: 6.2.1/Table6-2 printed6-7 PDF92,
6.2.4/Figure6-5 printed6-9/10 PDF94/95, 6.3.6 printed6-14 PDF99. Vector10,
ordinary six-byte SR/PC exception frame and unimplemented A-word exceptions.
[Official successor PDF](https://www.nxp.com/docs/en/reference-manual/MC68000UM.pdf).
1992 M68000PRM: MOVEQ4-134 PDF238, NOP4-147 PDF251, RTS4-169 PDF273,
RTE6-84 PDF538; instruction/encoding/register/flag prose only.
[Official successor PDF](https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf).
Access 2026-10-12; eligible published ISA documentation, extends SRC-0069/0079.
Downloads failed at publisher, web parsing reviewed; no local hash/successful
visual-page review claimed. No examples or implementation from these manuals.
This is generic 68000 ISA evidence, not January1984 OS behavior.

## SRC-0094 - Independent A-line fixture and boundary

Original SPEC-0021 / ADR-0024 / IMPL-0025 / TEST-0029, portable C interpreter,
shared value-frame boundary and original 68000 bridge/wrapper. Common twelve-byte
fixture generated independently from documented encodings. Codex-assisted lead
self-review; human review pending. No external implementation or assets.

## SRC-0091 - Shared C service probe observations

2026-10-12, SPEC-0020 / TEST-0028, compiled source
`02e2f5a6daf29a1fd4dc7a61ea460d371fa493ee`. Three fresh isolated PC starts and
two fresh Snow 128K profiles report ten original service checks, zero errors and
signature 53564331. [Detailed report](../docs/development/shared-services-evidence.md).
Only own post-success PC registers and own Mac RAM/scene observed; retain forty
Mac diagnostic bytes, no firmware/code/full dumps or protected pixels. Exact
cleared platforms retained; prior machines/media unchanged. CI 38257185223 passes
both jobs and matches all six EFI/two ROM profiles/source fingerprints. Original
project evidence, not January 1984/physical/complete OS compatibility.

## SRC-0088 - Owner continuation to shared OS services

Owner, 2026-10-12: "OK, let's do it" after the completed PC fault checkpoint
and proposed shared OS services for Mac and PC. Authorizes original contracts,
bounded implementation/probes/tests and normal evidence/HQ Git publication.
The continuation go-ahead covers this slice's necessary bounded probe checks on
the existing cleared platforms; use one fresh isolated PC VM and fresh Snow
profiles, retaining prior machines/media and the no-firmware-internals boundary.
No Apple input, firmware internals, UART implementation review, delegation,
physical writing or OneNote. Full historical/edition gates remain separate.

## SRC-0089 - Published trap-address service interfaces, cross-check

2026-10-12, SRC-0068/0078 published March 1985 Promotional Edition, local
1,326-page scan SHA-256 `924902074a9798457949c46f83ac016c98b10f50258ef36c277dcd0f8278b237`.
Assembly chapter printed 8 (PDF106, Figures 2/3 visually reviewed), 9, 12 and
14 (PDF107/110/112); OS Utilities printed 21-22 (PDF1178-1179). Scope: trap
word fields, per-routine calling convention warning, register preservation,
D0 word test, Get/Set parameters/results and installed-address semantics.
Published retail Volume III (SRC-0080), Appendix C III-222 (PDF229, visually
reviewed) confirms A146/A047. These are later-manual rules, not January 1984
confirmation. [Published scan](https://vintageapple.org/inside_o/pdf/Inside_Macintosh_Promotional_Edition_1985.pdf).
No sample caller instructions, equate files, compressed table representation,
dispatcher algorithm, assets or Apple implementation used. The original flat
32-bit address directory is our engineering choice, not an Apple layout.

SRC-0069 additional prose reviewed: Motorola 1992 programmer's manual TST,
printed 4-192 / PDF296: N/Z from operand, V/C clear, X unchanged. Official
[publisher-successor PDF](https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf).
No downloaded image/hash claimed; instruction prose only. Used for explicit
word-test semantics in the two-service C boundary, not CPU instruction execution.

## SRC-0090 - Original portable trap-address contract and fixtures

SPEC-0020 / ADR-0023 / IMPL-0024 / TEST-0028: independently authored native
C state, checked logical addresses, explicit service results and probes. Eligible
project artifacts; Codex-assisted, lead self-review, human provenance review
pending. No third-party implementation or copied assets.

## SRC-0087 - Own native terminal-fault observations

2026-10-12; SPEC-0019 / TEST-0027. Three UD and three GP cold starts plus one
normal control on one newly isolated exact cleared VirtualBox machine. Only own
post-success serial output and selected CPU registers observed, never memory or
CR2. [Report](../docs/development/fault-checkpoint.md) and original sanitized
manifest; compiled source `2f64689521fe57866b7a70572f2f3bb8a7bbb008`, CI
38250103771 passes both jobs and six image hashes match. The outer batch logging
syntax error occurs after all observers pass; preserve it separately. This is
project development evidence, not historical/physical/firmware certification.

## SRC-0086 - Original native fault fixtures and verification

Corresponding original SPEC-0019 / ADR-0022 / IMPL-0023 / TEST-0027 assembly,
tooling and own synthetic verification inputs. Project artifact, eligible within
this scope; Codex-assisted original design, lead self-review and human provenance
review pending. No external code/assets. Build checks do not prove native delivery.

## SRC-0085 - Owner-authorized native PC fault checkpoint

- Owner conversation, 2026-10-12: "OK, do it" after the proposal to deliberately
  trigger a CPU error and verify our handler catches it and stops cleanly in
  the installed VirtualBox, before returning to shared OS services.
- Scope: SPEC-0019 implementation, verification, new isolated VM trial and
  ordinary evidence/HQ Git publication. Existing trial permissions persist.
  No physical writes, emulator internals, UART implementation work, delegation,
  Apple input, OneNote or edition completion is authorized.
- Additional SRC-0043 prose reviewed 2026-10-12: Intel SDM 3A 253668-090US,
  February 2026, sections 7.13, 7.14.2/4/5, 7.15 Interrupt 6 (#UD, printed
  7-31) and Interrupt 13 (#GP, printed 7-41/42). UD2 faults at its instruction
  with no CPU error; descriptor-limit violations produce #GP and identify the
  selector. Eight-byte saved fields and IST stack switching are CPU interfaces.
  [Publisher PDF](https://cdrdv2-public.intel.com/874249/253668-090-sdm-vol-3a.pdf),
  existing reviewed hash `42166ab4aeb53df119a794a1da36eaa8a9f123eea61d90e8fe6195da4ef6654f`.
  No sample implementation or emulator source inspected or copied.

## SRC-0084 - Own original ROM Snow trial observations

- Codex, 2026-10-11 local date (UTC observations on October 12), owner-directed
  SPEC-0018 trial. [Dated record](../docs/development/mac-rom-trial.md), own
  image/scene/counters only, exact SRC-0082 executable, Macintosh 128K model.
- Initial image at `7c673f79` reached the original scene and advancing VBL
  count with zero presses, but redraws were slow. Corrected source at `a4124acd`
  passes two timed cold starts and the owner-confirmed left-button input test
  with two observed presses. Own diagnostics show stage 4, one trap, expected
  image seal and zero fault. CI 38194014724 passes and its own ROM matches local.
  Follow-up completed 2026-10-12; provisional failures/limits remain in the record.
  No Apple ROM, disks, emulator source or historical software runs.
- Black-box development observation, not reference compatibility, physical
  boot, calibrated time or complete edition evidence. Human review pending.

## SRC-0083 - Own original ROM verification artifacts

- Codex, 2026-10-11, SPEC-0018 / IMPL-0022 / TEST-0026 and the
  [build/hosted snapshot](../docs/development/mac-rom-evidence.md). Original
  sources/fixtures only; 24 suites in four Clang/GCC profiles and four
  zero-divisor fault controls; Os/O0 68000 image twins, corruption/omitted-object
  controls, vector/span/helper/stack audits. Generated outputs remain outside Git.
- Initial checkpoint is unloaded verification, not a Snow run, historical API
  result, physical boot or whole-OS budget proof. Existing core/PC source bytes
  are unchanged. Any trial/CI results get separate dated observations.
- AI-assisted original implementation and self-review; human provenance review
  pending. No target library implementation, emulator source or Apple payload.

## SRC-0082 - Snow v1.5.0 Windows x86-64 black-box platform clearance

- Publisher release <https://github.com/twvd/snow/releases/tag/v1.5.0>,
  published 2026-07-06; Windows asset `Snow.Windows.x86_64.zip`, 9,423,852 bytes,
  SHA-256 `d98643b7da36ec1a9adb39b8ca06f317a61c4bd07c033a22e3438c16974d955a`,
  matches GitHub publisher asset digest. Archive contains only `snowemu.exe`,
  23,706,597 bytes, SHA-256
  `32cf5285582c0f509e4f1ac38d86bfb90440bae9da43b7823bfb0841dea588e6`.
- Release license metadata: MIT; LICENSE blob
  `a8abdfb415ddafc764b1e3530c96d5d4ae2a03e4` at the exact release tag. Reviewed
  2026-10-11: user documentation and publisher asset/license metadata only.
  No emulator source, disassembly, reference ROM or disk image retrieved.
- Cleared only as an uncertified black-box test platform for SPEC-0018 under
  SRC-0076. Not a product dependency, source input, historical oracle or
  transitive-license/provenance certification. Own ROM only, 128K model, normal
  speed, validation override, no persistent PRAM/disks/network, isolated app
  settings. Clearance precedes execution; actual observations belong in TEST-0026.
  Required mouse mode: Relative (hardware emulation). The default Absolute
  mode patches guest globals and is excluded from this own-ROM trial.

## SRC-0081 - Pinned Ubuntu GCC 13.3 68000 cross tooling

- Ubuntu 24.04 package metadata and downloaded binary packages, 2026-10-11;
  GCC/cpp/base/libgcc-header package version `13.3.0-6ubuntu2~24.04cross1`,
  binutils `2.42-4ubuntu2.10`, target libc headers `2.39-0ubuntu8.2cross1`,
  Linux headers `6.8.0-39.39cross1`, host libisl `0.26-3build1.1`, host libmpc
  `1.3.1-1build1.1`. Origin archive.ubuntu.com, exact package hashes in the
  [tool lock](../scripts/m68k-toolchain-lock.json).
- Eligible compiler/header/build tooling only. Relocated workspace copy runs in
  existing Ubuntu 24.04 WSL without installing system packages. Target runtime
  archives and implementation bodies are not read, linked or redistributed.
  GCC/GNU binutils licenses and compiler host libraries remain separate from
  ClasSICK's own GPL-3.0-or-later code. Product-runtime adoption is not approved.
- GCC option prose: <https://gcc.gnu.org/onlinedocs/gcc/M680x0-Options.html>;
  compiler integer-helper interfaces: <https://gcc.gnu.org/onlinedocs/gccint/Integer-library-routines.html>.
  No compiler/runtime source or sample algorithm used. Independently authored
  helpers and explicit -nostdlib/68000 assembly validation under SPEC-0018.

## SRC-0080 - Published Inside Macintosh III hardware scan, separate printing

- Apple Computer, Inside Macintosh Volume III, Addison-Wesley, copyright 1985,
  ISBN 0-201-17733-1; eleventh printing June 1990. Public scan at
  <https://vintageapple.org/inside_o/pdf/Inside_Macintosh_Volume_III_1985.pdf>,
  290 pages, 50,687,193 bytes, SHA-256
  `4c0de9a3a4d41c41b8462b2a77aad8b95b92e1980afff165c1f986da0a81988b`.
  Title/copyright/retail warranty identify a published edition, not a draft.
- Read 2026-10-11 using bundled pypdf and Poppler page rendering. Hardware prose
  and factual summary constants only: III-17-19, III-39-46 (PDF pages printed
  locator +7), visually checked the VIA/address tables. This is a different
  scan/printing from the owner's SRC-0071; it corroborates those hardware facts,
  not byte identity. No equate files, code samples or Apple algorithms adopted;
  independently declare cited numeric interface facts rather than copying text.
- Eligible published 128K/512K hardware interface; later-manual System 1 API
  applicability still needs confirmation. Copyrighted book stays outside Git;
  no pages or copied assets republished.

## SRC-0079 - Motorola MC68000UM reset and exception prose

- Motorola M68000 8-/16-/32-Bit Microprocessors User's Manual, publisher-successor
  copy <https://www.nxp.com/docs/en/reference-manual/MC68000UM.pdf>, accessed
  2026-10-11 through cached browser PDF text. Sections 6.2.1, 6.2.4, 6.3.1 and
  6.3.5, printed 6-4, 6-9 and 6-11-13. Scope: actual MC68000 reset SSP/PC,
  supervisor/mask state, vector locations and ordinary TRAP exception return.
- Eligible published CPU-interface prose; distinguish MC68010-specific VBR and
  frames, which are not adopted. Complements SRC-0069's family reference.
  Direct local download was blocked by publisher delivery; no local-file hash
  or visual-table verification is claimed. No sample code or implementation used.

## SRC-0078 - Synertek SY6522 published register references

- Synertek Inc., SY6522/SY6522A Programming Reference, W-5K-9/80 (September 1980),
  <http://www.6502.org/documents/datasheets/synertek/synertek_sy6522_programming_reference.pdf>,
  7 PDF pages, 585,409 bytes, SHA-256
  `96f0d696218a4cebb1124f555dbb13774d125d3f735ab7630d1418fc047e92fe`.
  Title/publisher and internal-register/port/interrupt tables visually checked;
  PDF pages 2/3/6 correspond to printed 2-3/4-5/10-11. AN5 corrects the earlier
  reference's IER bit-7 read description; this profile never reads IER.
- Synertek Applications Information AN5, March 1982, copyright 1982, publisher
  reprint notice, <http://www.6502.org/documents/datasheets/synertek/synertek_sy6522.pdf>,
  12 pages, 5,920,052 bytes, SHA-256
  `cd617332a41f6820a50138c8f45883119f0df6ce710206b607c304142746d7f1`.
  Section 1/Figure 1 (page 2) confirms CA1 clear and enable semantics. A separately
  archived GIF reprint corroborates the title/table but is not the same PDF.
- Eligible published hardware-interface descriptions, not code examples.
  Read 2026-10-11 through Poppler page images; documents stay outside Git.
  No Linux driver, confidential draft, emulator source or sample code is an input.

## SRC-0077 - Snow published user documentation and license metadata

- Thomas W. / Snow, <https://snowemu.com/>, <https://docs.snowemu.com/> and
  <https://docs.snowemu.com/manual/starting>, reviewed 2026-10-11. Publisher
  describes 128K support, hardware emulation without ROM hooks, and a Load ROM
  advanced option disabling ROM validation for modified images. Eligibility is
  user interface documentation only; these statements are not observed startup.
- MIT license identity checked through publisher release/license metadata.
  No repository implementation, patch code, debugger static-global catalogs or
  reference payload read. Exact binary clearance is SRC-0082; no OS dependency.
- Additional published user-manual review: Keyboard/mouse sections and
  Workspaces at <https://docs.snowemu.com/print>, 2026-10-11. Absolute mouse
  mode patches guest globals and is unsuitable for non-original software;
  Relative hardware mode is selected before the first guest launch. This
  qualifies the introductory no-hooks statement; it is not a blanket guarantee
  for every default setting. Workspace data is saved by Snow's user interface
  and inspected only as configuration, never as emulator implementation.

## SRC-0076 - Owner directs own-ROM startup in Snow

- Owner, 2026-10-11: reaffirmed PC/Mac ports without an Apple-supplied ROM and
  delegated the practical path; allowed recommendations of other emulators.
  After the proposal to build our replacement ROM and test it in Snow, answered
  "OK, do it!". The lead stated continuation strictly from published manuals
  after the discovery audit. Scoped continuation includes own specification,
  original code/tests, tool preparation, exact black-box platform clearance and
  isolated Snow trial, ordinary commits/pushes and HQ Git wiki plans.
- No emulator fork, source inspection, delegation, new chats, automation,
  Apple payload, physical writing or OneNote operation. Human provenance review
  remains pending; the continuation is not a legal/two-team certification or
  waiver permitting uncertain material. Size budgets are deferred, not dropped;
  real Macintosh/Mini vMac and full edition/parity gates remain separate.

## SRC-0075 — Own Mini vMac admission review and installed tool capability

- Codex, 2026-10-11, [admission review](../docs/research/mini-vmac-admission.md),
  from public `7d2c4bf514cd92bada3cdf2b1a07eb9b0b87061d` and SRC-0074.
  Original documentation only; no emulator download/run/source inspection,
  ROM access or new boot code. Exact platform clearance and trial approval absent.
- Installed pinned LLVM-MinGW 20260908: Clang reports 23.1.1 and no M68k
  registered target; `llc.exe` is absent. Tool capability observation, not a
  compiler adoption or 68000 build result. The owner asked which Mini vMac to
  provide and where; this is not a go-ahead to run it.
- Publisher-documented ROM patching triggers the supplied handoff's stop for
  owner decision. Originality/provenance self-review only; human review pending.

## SRC-0074 — Gryphel Mini vMac publisher documentation

- Paul C. Pratt / Gryphel Project; reviewed 2026-10-11 through browser-extracted
  text: [stable 36.04 download](https://www.gryphel.com/c/minivmac/dnld_std.html),
  [hardware reference](https://www.gryphel.com/c/minivmac/hardware.html),
  [options](https://www.gryphel.com/c/minivmac/options.html),
  [developer options](https://www.gryphel.com/c/minivmac/develop.html),
  [variation directions](https://www.gryphel.com/c/minivmac/var_dir.html).
- Eligible only as published user/build interface documentation: hardware ROM
  loading/checksum/patching and Floppy Drives; options Model, Memory, ROM Size,
  Check ROM Checksum, Disable Rom Check, Disable Ram Test and Minimum Extensions;
  stable version/model and variation service conditions. Pages dated 2020;
  exact installed variation behavior still needs qualification. Copyrighted
  prose is cited and paraphrased, not republished; no source code, patch bodies,
  binaries, Apple payloads or linked implementation material read or adopted.
- This is documentation eligibility, **not** clearance of any Mini vMac binary.
  Findings and unresolved platform admission: [review](../docs/research/mini-vmac-admission.md).

## SRC-0073 — Owner priority: Mini vMac first boot, size later

- Owner, 2026-10-11, after the 128K feasibility study: "For right now, all I
  have access to is Mini vMac, and the number one priority is to get this thing
  booting, regardless of the size. We can work on that later." He then handed
  the next work to ChatGPT.
- Requirement only. The first M0.3 evidence target becomes a cold start of our
  own ROM image in the owner's Mini vMac; RAM/ROM size budgets are deferred,
  not dropped (the charter's 128K requirement and its scope-decision rule
  persist). No real Macintosh hardware is available now. Running Mini vMac still
  needs a recorded black-box clearance and the owner's go-ahead; no Apple ROM
  may take part in the boot.

## SRC-0072 — BMOW Mac ROM-inator product page (third party)

- Big Mess o' Wires, "Mac ROM-inator" product page,
  <https://www.bigmessowires.com/mac-rom>, read 2026-10-11 through the WebFetch
  tool's extracted text (HQ lead CS-SRC-0035).
- Claims used, each a vendor statement still to be verified on hardware: the kit
  replaces the stock 64K or 128K ROM in a compact Macintosh with 1 MB of flash
  (132K ROM code, 28K startup sound, up to 864K ROM disk); it supports the Plus,
  512Ke, 512K and 128K; "128K ROM code turns a Mac 128K or 512K into a 128Ke or
  512Ke"; its flash tool needs 192K of RAM, so reflashing on a 128K needs a second
  Macintosh or an external programmer.
- Eligible only for those hardware-capacity claims. The kit's ROM code is
  patched Apple ROM code, and the page links to Apple ROM listings: never
  inputs; their links were not followed. The page says the board design is
  open source; its license and design files were not reviewed. No adoption.

## SRC-0071 — *Inside Macintosh* Volumes I–III (Addison-Wesley, 1985), owner's copy

- Apple Computer, Inc., *Inside Macintosh*, Volumes I, II and III, Addison-Wesley,
  copyright 1985; ISBN 0-201-17731-5 (I), 0-201-17732-3 (II) and 0-201-17733-1
  (III). The copy's printing lines read ninth printing, December 1988 (II) and
  twelfth printing, November 1990 (III); Volume I's printing line is not in the
  scan. Locators are printed page numbers of this copy.
- Owner, 2026-10-11, asked whether lawful copies were available, answered "Yes.
  I have had them for many years. Here they are." and supplied one combined PDF:
  1,284 pages, 19,245,372 bytes, SHA-256
  `b65fe8efe782799a333a0b59b225e2bbaf9e2e150dc949dd555571971b331d97`; PDF
  metadata name ABBYY FineReader 9.0 and 2008-11-18. Kept outside Git; not
  committed, uploaded or republished. Reviewed by Claude 2026-10-11 through local
  text extraction (poppler `pdftotext -layout` 24.02.0, SHA-256
  `381d4d79034f00c04e88d1b92155153543b1895605c21946d643a9d965bb10d9`) and page
  images wherever a value was recorded.
- Published: a retail book with ISBNs, "simultaneously published in the United
  States and Canada", with a retail-purchase warranty; no confidential marking or
  publication disclaimer. Eligible under SRC-0067 for interface prose,
  data-structure descriptions, the documented global-variable locations
  (Appendix D, III-227–231) and the Macintosh 128K/512K hardware chapter
  (III-15–46). Sample code and Apple equate/definition files are not inputs;
  constants printed in chapter summaries are cited as documented facts, never
  copied as source. Apple copyright; cited, not reproduced.
- Applicability: the book describes ROM version 105 in the Macintosh 128K and
  512K and the System file created May 2, 1984 (I-4), about four months after
  System 1. Later printings may carry corrections. Every fact still needs
  confirmation for the January 1984 profile (R-001) before a specification
  adopts it. See [System 1 sources](../docs/research/system1-sources.md) and the
  [low-memory map draft](../docs/research/system1-low-memory.md).

## SRC-0070 — Excluded: Apple pre-publication Macintosh drafts (1984–85)

- Excluded by owner decision SRC-0067 (2026-10-11); never an input for
  specifications, tests, code or behavioral claims. Recorded here so the exact
  items stay identifiable. All are bitsavers scans on archive.org, identified by
  archive.org file SHA-1 (not downloaded):
  - *Inside Macintosh* Vol 1 1984 binder, item `bitsavers_applemacIn84_27699101`,
    `Inside_Macintosh_Vol_1_1984.pdf`, 27,699,101 bytes, SHA-1
    `b96770af19a7daec610b4710108c10d8b6a8cf97`.
  - *Inside Macintosh* Vol 2 1984 binder, item `bitsavers_applemacIn84_23664048`,
    `Inside_Macintosh_Vol_2_1984.pdf`, 23,664,048 bytes, SHA-1
    `5bb8e1411dc4e9c17d6ae1aa068402cb1c47a794`.
  - *Programming Macintosh Applications in Assembly Language*, first draft
    2/27/84 (a bound compilation of several draft chapters), item
    `bitsavers_applemacdeogrammingMacintoshApplicationsinAssembly_29406178`,
    29,406,178 bytes, SHA-1 `06e0d34b4c7b29add86bcf93414dc553777a7b1d`; the
    dossier's kuiper/bitsavers mirror URL names the same document.
  - *Inside Macintosh*, "The Macintosh Hardware", second draft 2/13/85, item
    `bitsavers_applemacInre198502_22173505`, 22,173,505 bytes, SHA-1
    `7b12178d66821efbf980555c0ed25efc215a3a93`.
- Eligibility concern (sanitized): each copyright page states that distribution
  of the draft in limited quantities does not constitute publication. Binder and
  assembly-draft pages carry a running "CONFIDENTIAL" footer, and several
  chapters are marked review or preliminary drafts "not for distribution". The
  hardware draft has the publication notice but no confidential footer. Binder
  chapter dates run from 1982 to July 1984. POLICY admits published
  documentation; public scanning does not make these drafts published.
- Exposure: the 2026-10-07 research dossier (Claude, separate chat) read
  interface prose of the assembly draft (about pp. 5–22) and the hardware
  draft's OCR text. On 2026-10-11 Claude read only the OCR title pages,
  modification histories, copyright notices and running footers of all four to
  assess eligibility; derived OCR text SHA-256 `3c9f325a…0c36e` (Vol 1),
  `c9a06d7c…e981cf` (Vol 2), `2080565e…c46d33` (assembly draft) and
  `97c507ab…c6ea678` (hardware). Dossier facts traced to these drafts must be
  re-sourced from an eligible publication before any use. No content from them
  is in this repository.

## SRC-0069 — Motorola M68000 Family Programmer's Reference Manual

- Motorola Inc., *M68000 Family Programmer's Reference Manual (Includes CPU32
  Instructions)*, 1992; publisher-successor copy at
  <https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf>, front matter
  checked 2026-10-11. Appendix B (Exception Processing Reference, printed B-1
  onward) includes the MC68000 bus/address-error stack frame (Figure B-2, B-3).
- Eligible published ISA manual for R-009 and the 68000 exception model. Not yet
  downloaded or hashed; record the SHA-256 and exact section locators when a
  local copy is reviewed for a specification. No sample code adopted.

## SRC-0068 — *Inside Macintosh* Promotional Edition (Apple, March 1985)

- Apple Computer, *Inside Macintosh*, Promotional Edition, with "A Letter from
  the Macintosh Developers Group" dated 15 March 1985. Bitsavers scan, archive.org
  item `bitsavers_applemacInionalEdition1985_53888388`, file
  `Inside_Macintosh_Promotional_Edition_1985.pdf`, 53,888,388 bytes, SHA-1
  `4ed5b3c20289835996b160d7ab3a628f497f13c5`, MD5
  `d3822276bc5f99a0e3e3beb69b609cbe` (archive.org metadata; PDF not
  downloaded). Reviewed 2026-10-11 through archive.org's derived OCR text,
  2,468,477 bytes, SHA-1 `f891c8ee9f6c74dc98cb1a427f300b0c76dcf781`, SHA-256
  `77dca9ebd77b9f029f954b9e98dc247e224c6ec9f566d3701b4f72fce3b5dfd9`.
- Published: the warranty page says the manual is "sold as is" to "you, the
  purchaser", and the letter says this inexpensive edition meets demand until
  the final edition reaches bookstores. No confidential markings or publication
  disclaimer were found in the OCR text. Apple copyright; linked, not
  republished.
- Eligible for interface prose and data-structure descriptions of the 64-KB-ROM
  Macintosh; sample code and Apple equate/definition files are not inputs.
  Applicability: a later manual relative to January 1984. Its own index dates
  each manual from 3/2/83 to 1/31/85; only QuickDraw (3/2/83) predates System
  1's January 1984 release. See
  [System 1 sources](../docs/research/system1-sources.md) for the per-manual table.
  Every 1984 behavioral claim still needs owner-held reference confirmation.
  It gives symbolic global names and sizes, not addresses; the published
  address source is SRC-0071.

## SRC-0067 — Owner decision: published editions only

- Owner, 2026-10-11, asked whether to use Apple's 1984–85 pre-release drafts
  (marked "CONFIDENTIAL" and "does not constitute publication") or only
  published editions, chose "Published editions only (Recommended)": the March
  1985 Promotional Edition, the 1985 *Inside Macintosh* books and Motorola's
  manual, with 1984-specific details confirmed later on real hardware.
- Binding eligibility ruling for this project; see SRC-0070. Requirement only.

## SRC-0065 — Owner's qualification-candidate VM trial approval

- Owner, 2026-10-11, after the SPEC-0016 continuation and read-only milestone
  review, answered "Go ahead" to three automatic cold starts and about two
  minutes of live Space input. This is approval for the new candidate, separate
  from SRC-0060's earlier payload trial.
- Covers fresh pinned Windows verification, a fresh isolated machine using
  SPEC-0015 settings and SPEC-0016 media, serial/own-scene/host-clock observations,
  operator input and ordinary public/HQ evidence and Git wiki publication.
- SRC-0059's no-firmware-internals boundary, existing owner/project VM protection,
  strict criterion 6, role separation and Macintosh/three-edition goals persist.
  No new guest/driver code, delegation, chats, automation, other emulator clearance,
  physical writing or live OneNote operation. Requirement, not a runtime result.
- The owner accepted the live candidate session, then confirmed:
  “I pressed Space several times and saw the scene change.” TEST-0025 retains
  the observed six initial presses; no exact-two-count claim is made.

## SRC-0066 — Project-owned qualification-candidate VM observations

- Own SPEC-0016 image observed 2026-10-11 under SRC-0065 / TEST-0025 and
  IMPL-0021; [sanitized evidence](../docs/development/qualification-vm-evidence.md).
- Three no-input headless starts and one live owner-operated start pass both
  alias probes, unchanged read controller bits and guest second 60/result zero.
  Own scene/serial and bounded public post-READY RIP/RSP observations are retained.
- Extends SRC-0061's same Oracle public management contract with getregisters,
  manual section 15.16.2.11: only own post-READY RIP/RSP, no memory/code dump.
  No external implementation, firmware internals or protected assets are inputs.
- Original output/host orchestration under GPL-3.0-or-later. Firmware vendor and
  revision are self-report, not an image fingerprint. Formal B2, historical
  compatibility, physical/edition/Mac goals and human provenance review remain open.

## SRC-0064 — Owner's delegation of the B2 qualification decisions

- Owner, 2026-10-08, after Claude's project-wide audit and prioritized queue:
  asked Claude to "figure out how you could be effective RIGHT NOW", deferring
  VM work to ChatGPT. Asked which items to start (including D1, the probe
  image): "I think you should decide. I am underqualified for this." On D2,
  criterion 6's firmware hash: "I don't know."
- Claude's decisions under that delegation: implement SPEC-0016's measurements
  in the normal native image (not a separate probe image), verify them in pinned
  CI and hand the VM trial to ChatGPT; keep criterion 6 as written, so B2 stays
  partial on the firmware hash; defer QEMU/OVMF/MAME platform clearance.
- Requirement only. Covers code, records and CI. It does not authorize a VM start:
  the next trial still needs the owner's explicit go-ahead in chat under
  AGENTS.md, and SRC-0059/0060 boundaries persist. No delegation, new chats,
  automation or live OneNote.

## SRC-0063 — Owner's Claude continuation after the first VM trial

- Owner, 2026-10-08, supplied a Codex-written continuation naming Claude
  (Anthropic) lead again. First assignment: a read-only, evidence-based audit of
  remaining formal B2 qualification: map each criterion to evidence; review the
  self-test/A20/machine-state precondition against eligible primary sources;
  determine whether permitted metadata can close firmware identity; prepare the
  smallest next qualification contract and any owner decision; separate
  out-of-gate items; propose the next bounded Macintosh research milestone.
- Requirement only. SRC-0059/0060 boundaries persist; no new guest code, VM
  launch merely to reproduce settled results, firmware extraction, delegation,
  new chats, automation or live OneNote. The prompt itself stays private.

## SRC-0060 — Owner's first isolated VM trial go-ahead

- Owner, 2026-10-08, responding to the explicit trial question:
  "Yes—prepare and run the isolated trial".
- Authorizes a reviewed isolated VM configuration and recorded owned-media attach
  procedure, first black-box execution/qualification, timing/input/serial/screen
  evidence, repeat cold starts and ordinary project publication/HQ Git planning.
- SRC-0059 conditions persist. No firmware extraction/study, Apple material,
  existing-VM modification, physical disk writing, delegation, new chats,
  automation or live OneNote. Macintosh/mini vMac and three-edition goals persist.
- OpenAI Codex (GPT-6) resumes implementation leadership from the owner's Claude
  continuation prompt. Requirement only; not evidence of a boot or compatibility.
- The owner then accepted a live VM keyboard test and confirmed cold-04:
  “I tapped Space twice and saw the scene change.” This is scoped operator
  observation, corroborated by our own Space counter, not historical parity.

## SRC-0062 — Project-owned native VM observation artifacts

- Own unchanged SPEC-0014 disk and SPEC-0013 native image observed 2026-10-08
  under SPEC-0015 / TEST-0022; [sanitized evidence](../docs/development/vm-trial-evidence.md).
- Six uninterrupted cold starts reach the native 60-second endpoint; a separate
  synthetic Escape trial checks early stop. cold-04 has owner-confirmed live
  Space input. Local original-scene captures and own serial/host-clock records
  are retained with hashes; only own text and sanitized metadata are published.
- Original project output, GPL-3.0-or-later. No protected reference system,
  firmware pixels/internals, implementation input or Apple material. Formal B2
  qualification and human provenance review remain incomplete; no edition,
  physical hardware or historical Macintosh compatibility claim.

## SRC-0061 — Installed VirtualBox public management interface

- Oracle VirtualBox 7.2.16r174877 public VBoxManage usage observed 2026-10-08,
  scoped createvm, modifyvm, modifynvram Secure Boot configuration, convertfromraw,
  clonemedium, storagectl/storageattach, showvminfo, startvm/controlvm screen,
  scan-code input and power-off operations. Reuses SRC-0035's exact publisher
  [manual](https://download.virtualbox.org/virtualbox/7.2.16/UserManual.pdf),
  sections 15.7, 15.12/15.13, 15.15, 15.34/15.35, 15.44, 15.47–15.49; used locators and
  executable/usage hashes are retained with TEST-0022.
- Only public command contracts and existing primary manual text are reviewed.
  No tool/firmware/device implementation body, dump or external driver code.
  Native machine behavior is not inferred from help text or model names.
- Earlier COM access denial did not recur in the approved management session.
  Unsupported subcommand `--help` probes returned usage with nonzero exits;
  they are retained as discovery attempts, not successful trial commands.

## SRC-0059 — Owner's firmware test-platform decision

- Claude reported the documents-only firmware result on 2026-10-08: no permitted
  document can bind the installed VirtualBox 7.2.16 x64 EFI image to its published
  source (only a rebuild could, which stays excluded), while the binding policy
  governs what enters System 0.97, not the platform it runs on. Offered: clear it
  as a black-box test platform, keep it blocked, or seek another firmware. The
  owner chose "Test platform only (Recommended)".
- Scope: VirtualBox 7.2.16 x64 EFI may serve only as a black-box execution
  platform for our own images, under the conditions in the
  [firmware decision](../docs/development/firmware-eligibility.md). It does not
  certify the firmware's provenance or rights, adopt or redistribute it, permit
  extraction or study of its internals, or authorize a VM trial; that needs a
  separate explicit owner go-ahead. Physical PCs' vendor firmware is treated the
  same way when those gates come. Requirement only; no edition claim.

## SRC-0056 — Owner's boot-media continuation and FAT32 decision

- Owner, 2026-10-08: "ok, do it." after Claude proposed the two preparation tracks
  that need no VM: original FAT/UEFI media packaging with independent checks, and a
  documents-only firmware eligibility review. Ordinary specification, code, tests,
  provenance, commits/pushes and HQ registration under the existing workflow.
- Asked whether Claude could fetch Microsoft's FAT32 specification through the
  owner's desktop browser pane, the owner chose "Yes, download it". Asked to accept
  its embedded agreement, the owner asked about alternatives (Linux FAT support was
  explained as excluded implementation code; ECMA-107/FAT16 was offered), then
  answered: "Our goal is to only clean room system 0.97. Adding FAT32 support is
  outside of that. I think that's acceptable."
- Recorded reading, stated back to the owner in chat with an offer to switch to
  ECMA-107/FAT16: the owner accepts the Microsoft specification terms for this PC
  boot tooling, and the clean-room requirement governs the System 0.97
  reimplementation rather than published PC interface specifications used under
  their own terms. No correction had been received at publication. This does not
  relax the no-Apple-material rule, permit reading other implementations, waive
  firmware eligibility or authorize a VM, native run, delegation or OneNote.
- Claude (Anthropic; session configured as `claude-opus-5-5`) leads this work.
  Human provenance/rights review pending; this is not legal advice.

## SRC-0057 — Microsoft FAT32 File System Specification

- Microsoft, *Microsoft Extensible Firmware Initiative FAT32 File System
  Specification — FAT: General Overview of On-Disk Format*, Version 1.03,
  December 6, 2000; legal agreement updated March 30, 2011. Publisher copy
  <https://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/fatgen103.doc>
  (UEFI 2.11 "Links to UEFI-Related Documents" names this document), fetched
  2026-10-08 into the owner's desktop browser pane: 222,720 bytes, SHA-256
  `b17d66c796d9cd3070adf4ccbc00add5a8b6b6f5491fe8ea949df778fce60172`, served
  Last-Modified 17 Feb 2025. Text was extracted in the page for review; the
  document was not saved to the repository or vendored.
- Reviewed: Boot Sector and BPB tables (offsets 0–35), FAT32 structure at offset
  36, sector signature notes, FAT data structure and cluster/sector arithmetic,
  FAT type determination, FAT32 28-bit entries/EOC/bad mark, FAT[0]/FAT[1], FAT
  volume initialization (FAT32 sectors-per-cluster table and FAT size
  computation), FSInfo and backup boot sector, 32-byte directory entries, name
  rules, attributes, dot/dotdot, date/time formats and size limits. Long-name
  sections were scanned only to confirm short names suffice; not implemented.
- The document's C code fragments were read as interface description; none were
  copied. The SPEC-0014 writer and both checkers are original.
- Agreement: royalty-free copyright license and patent covenant limited to
  products that comply with the unmodified specification and are used for UEFI
  boot, install, setup, repair, diagnostics or inventory purposes; both terminate
  if the user initiates patent litigation against Microsoft or covered parties;
  export compliance applies. Accepted by the owner under SRC-0056. Scope fit and
  rights are pending human review; no Microsoft code or boot code is used.

## SRC-0058 — UEFI Forum published media and partition interfaces

- *UEFI Specification* 2.11 (same publisher PDF and SHA-256
  `a64b8e442004b91becc3de9afaf8ca61b259a9a3b436accb6b3711ab5400cee9` as
  SRC-0034/0038), reviewed 2026-10-08 from the owner's local reference copy by
  text extraction in the owner's Cowork Linux shell.
- 4.2 CRC note (PDF 174); 5.1–5.3.3 with Tables 5.1–5.8 (printed 111–120 /
  PDF 195–204): legacy and protective MBR, GPT overview, header, entry array,
  ESP type GUID, attributes and alignment guidance; 13.3–13.3.4.3 (462–467 /
  PDF 546–551): FAT variants, system partition, names, directory structure,
  partition discovery and media formats; Appendix A Table A.1 (1970 / PDF 2054)
  GUID storage. The removable-media file name is SRC-0038's 3.5.1.1 review.
- Eligible public interface prose and tables. Adjacent structure declarations
  were visible, not copied. Does not establish any firmware's actual FAT/GPT
  behavior, VirtualBox media handling or historical Macintosh behavior.

## SRC-0055 — Owner's readiness audit request

- Owner, 2026-10-08: "i typed 1 in error. Please perform the audit."
- Scope: audit current retained evidence, native-PC boot readiness and provenance
  prerequisites; record findings under the existing documentation/publication
  workflow. The accidental numeric reply grants no additional work or delegation.
- Requirement only. No firmware waiver, VM/native execution, additional agent,
  new chat, automation, live OneNote or change to the real Macintosh/mini vMac
  and three-edition requirements. TEST-0020 is aggregate evidence review;
  UART implementation boundaries and human review remain separate.

## SRC-0053 — Owner's single fresh UART implementation-agent authorization

- Owner, 2026-10-08: continue the missing native boot components with exactly
  one fresh implementation agent for bounded polling UART diagnostics; no inherited
  conversation/search history. Specifications, implementation, original fixtures,
  audits, provenance and ordinary commits are authorized under the existing scope.
- Requirement only; firmware eligibility remains needs-review. No native/VM
  execution, additional agents/chats, automation, live OneNote or change to the
  independent real Macintosh/mini vMac and three-edition goals. Implementation
  work is separately recorded from the lead's HQ/publication responsibility.
- OpenAI Codex (GPT-6) assisted; fresh context and self-review are not a legally
  certified or independently staffed clean-room procedure. Human review pending.

## SRC-0054 — TI TL16C550C published UART interface

- Texas Instruments, *TL16C550C Asynchronous Communications Element with
  Autoflow Control*, SLLS177I, March 1994, revised March 2021. Publisher
  <https://www.ti.com/lit/ds/symlink/tl16c550c.pdf>, directly reviewed from the
  owner's local reference copy 2026-10-08. SHA-256
  `3ef2d68733a8599cc2d0d7300b69c3530bc714de3ebb0fcfda03df60a4c7694e`.
- PDF/printed 21–23, Tables 7-1/7-3 register selection/DLAB/bit declarations;
  7.7.2 on 24 (FCR); 7.7.4/5 on 25 (polling/IER); 7.7.7 on 27 (LCR,
  Tables 7-6/7); 7.7.8 on 28 (LSR/error-read/THRE/TEMT); 7.7.9 on 29
  (MCR/Table 7-8); 7.7.11 on 31 (baud equation/Table 7-9); 7.7.13/14 on
  32 (SCR/THR). Selected prose and register tables were visually reviewed using
  PDFium images in the ignored owner workspace; pypdf extraction corroborated it.
- Adjacent block/reset/FIFO interrupt/IIR/MSR/RBR and clock circuit declarations
  were visible, unused beyond contextual read-only interface review. No code
  samples, third-party drivers/headers/OS implementations, search engines or
  firmware bodies were consulted. Initial extraction hit a local Windows encoding
  error; UTF-8 extraction then succeeded. Document/images are not vendored.
- Eligible scoped hardware interface only, copyrighted documentation linked rather
  than copied. Does not establish universal 16550 support, VirtualBox device
  implementation, PC address decoding, clock or native delivery. SPEC-0013's
  proposed 0x3F8/1.8432-MHz profile, scratch/readback checks, queue, time/call
  limits and conservative terminal error policy are original design decisions.
  No historical Macintosh behavior or external implementation/license adoption.

UART exposure boundary: existing eligible project contracts and owned source,
SRC-0052's already reviewed original byte-port instructions and SRC-0046 timer
are reused. The fresh implementer did not retrieve uncertain source or firmware.
Provenance eligibility and human rights review remain distinct from test success.

## SRC-0048 — Owner's return-to-Codex keyboard handoff

- Owner-supplied continuation, 2026-10-08, from Claude's verified handoff:
  resume SPEC-0012 through specification, implementation, verification and normal
  documentation/commit/push publication. Real Macintosh/mini vMac, all three
  editions and firmware provenance gates remain required. No delegation, new
  chats, automation, OneNote publication or firmware waiver is authorized.
- Requirement only. OpenAI Codex (GPT-6) is the implementation assistant for this
  checkpoint; implementation-lead self-review and human review remain distinct.

## SRC-0049 — Published PS/2-mode controller interfaces

- SMSC, *KBD43W13 Keyboard and PS/2 Mouse Controller*, undated 22-page edition,
  manufacturer-issued datasheet archived at
  <https://k.lse.epita.fr/data/8042.pdf>, accessed 2026-10-08. SHA-256
  `20f990798e61f3f876864a6e14667bd907d8c7c4ec30718b2acf85a5551ae678`.
  Reviewed buffers on printed/PDF 8, PS/2 status on 10, configuration and
  commands on 12, and AB/AD/AE interfaces on 13. AT-mode status meanings on 8
  are not substituted for the PS/2-mode contract. No internal implementation,
  firmware, third-party header or driver is an input.
- Holtek, *HT6542B Keyboard Controller with PS/2*, November 30, 1995, printed/PDF
  5 power-on self-test/status prose (55 success). Manufacturer-issued archived copy
  <https://telcontar.net/KBK/Holtek/docs/Holtek%20HT6542B%20datasheet.pdf>, accessed
  2026-10-08; SHA-256
  `27f40a0e58c9b476f2b39fbc51c9ec433686f6019a9b724997f91ef061d7910f`.
  Corroborates the success code for that chip's power-on self-test; the explicit
  AA command comes from SMSC. Requiring 55 after AA is the original qualified
  profile's acceptance condition, not a documented universal-device guarantee.
  This evidence does not establish another
  device's implementation or VirtualBox conformance. SPEC-0012 is a qualified
  compatible-device profile, not a universal i8042 claim.
- Copyrighted interface documentation is linked, not republished. Selected-page
  text/visual review occurred in the ignored owner workspace. No sample code used.
- TEST-0023 addendum (2026-10-08, Claude, text extraction of the same hashed
  copies): SMSC PDF 11 PS/2-mode output port (bit 0 System Reset, bit 1 Gate A20)
  and PDF 12–13 command D0 Read Output Port; no stated self-test effect on the
  output port. Holtek PDF 5: successful self-test registers 55, sets the system
  flag and enables the keyboard interface. No A20 statement for self-test.
- SPEC-0016 addendum (2026-10-09, Claude, same hashed copy): PDF 11 also defines
  output-port bits 2–7 (auxiliary data/clock, keyboard and auxiliary buffer-full
  interrupts, keyboard clock/data), which is why only bits 0–1 are compared. D1
  Write Output Port and F0–FF Pulse Output Port (PDF 12–13) are never issued.

## SRC-0050 — Published PS/2 keyboard protocol and set-2 codes

- Holtek, *HT82K629B USB + PS/2 Keyboard Encoder*, revision 1.10, August 30,
  2022, publisher <https://www.holtek.com.tw/webapi/11842/HT82K629Bv110.pdf>,
  accessed 2026-10-08; SHA-256
  `0fcc9cb8cb394c8f709edfec9f47bf5a92ffd3b06eb8823bfb71d9098b690979`.
  Printed/PDF 4–6: reset/BAT, ACK/resend/error, F5/F0/02/F4 commands and timing;
  10: Space/Escape set-2 make/break bytes; 11: extended/Pause prefixes.
  Hardware interface prose/tables only; adjacent USB/circuit/unused commands
  are not adopted. The protocol does not establish historical Macintosh behavior.
- SPEC-0012's timeouts, two retries, two-key scope and parser recovery are
  original policy choices. No third-party implementation or asset is copied.

## SRC-0051 — ACPI controller-present indication

- Extends SRC-0036/0046's pinned ACPI 6.6 publisher PDF (same SHA-256):
  section 5.2.9.3, Table 5.11, PDF 196 / printed 125, IAPC_BOOT_ARCH bit 1;
  Table 5.9's field offset 109. Reviewed 2026-10-08, including visual page review.
  Eligible interface declarations; no actual firmware table or parser used.
  A declaration is a prerequisite, not proof of controller readiness or rights.

## SRC-0052 — Intel byte-port instruction interfaces

- Extends SRC-0039/0043: Intel SDM volume 2A, order 253666-093, September 2026,
  IN, PDF 573–574 / printed 3-455–456, same pinned hash
  `87c5acb6f27e24d9d364841a0d2346c91a8482e36f403409954e2d2a8c817bac`.
- Intel SDM volume 2B, order 253667-093, September 2026, OUT,
  PDF 179–180 / printed 4-171–172, publisher
  <https://cdrdv2-public.intel.com/929354/253667-093-sdm-vol-2b.pdf>, accessed
  2026-10-08; SHA-256
  `a261998ace8e07f624bf3e2486bb6cfe950fb8be8d0dc9631bcecb4c5ec953e4`.
  Opcode EC/EE, AL/DX operands and privilege/exception interface only. Original
  Microsoft-x64 register marshaling; no other assembly implementation consulted.

Discovery exposure for SRC-0049–0052: search results showed OSDev/Wikipedia
summaries and a NURVE Chameleon user-guide snippet containing driver/API examples.
Those snippets were not adopted or studied as implementation inputs; the full
guide and OS code were not opened. SMSC KBD42W11, Microchip LPC47M172 and Holtek
HT6542B datasheet discovery was limited to published hardware interface pages.
Only the catalogued scoped contracts support SPEC-0012. No Apple implementation,
executable, ROM, copied asset or uncertain firmware body was retrieved or used.

## SRC-0047 — Owner's progress-loop continuation

- Owner, 2026-10-08: "do it." This came after the SPEC-0010 handoff named the
  60-second timed progress loop next. It authorizes SPEC-0011, tests, provenance
  and normal publication under SRC-0044. No external source is added.
- Requirement only; no firmware waiver, VM launch, OneNote or edition claim.

## SRC-0045 — Owner's PM-timer continuation

- Owner, 2026-10-08: "Do it." This came after the lead's SPEC-0009 handoff named
  the ACPI PM timer as the next concrete task. It authorizes SPEC-0010
  discovery/timer work, tests, provenance and normal publication under SRC-0044.
- Requirement only; no firmware waiver, VM launch, OneNote or edition claim.

## SRC-0046 — ACPI 6.6 system description table interfaces

- Same pinned publisher PDF as SRC-0036 (SHA-256
  `8c7542dd4de974ae47bba71bb0336637fe1e3838daad7692370ab4cf218efd35`), accessed
  2026-10-08 from the owner workspace copy. Sections 5.2.3.2 / Tables 5.1–5.2
  (GAS, PDF 172–174), 5.2.5.2–5.2.5.3 / Table 5.3 (UEFI RSDP location and GUIDs,
  RSDP fields, PDF 175–176), 5.2.6 / Table 5.4 (header, PDF 176), 5.2.8 / Table
  5.8 (XSDT, PDF 181), and 5.2.9 / Table 5.9 offsets 0–131 and 208–219 (FADT
  PM_TMR_BLK, PM_TMR_LEN, Flags, X_PM_TMR_BLK, PDF 182–188). Table 5.10 bits 8
  (TMR_VAL_EXT) and 20 (HW_REDUCED_ACPI), PDF 190–194.
- Text was extracted locally with pypdf into the owner's ignored workspace.
  Eligible published interface tables/prose only. No ASL, sample OS code,
  firmware tables or third-party parser consulted. The original policy (ACPI 2.0+
  only, one FADT, I/O-space timer, map-type checked reads) is narrower than ACPI.

## SRC-0044 — Owner's Claude implementation-lead handoff

- Owner, 2026-10-08: a written handoff that made Claude (Anthropic) the 0.97
  implementation lead from the verified state at 2bceffc / HQ 2e160cc. It
  authorizes continuing the missing B2 boot components, specifications,
  implementation, tests, provenance, documentation, commits and pushes. It
  recommends native framebuffer presentation and an original post-handoff scene
  next. Later the same day, after the lead proposed Linux-VM development checks
  plus pinned Windows gates, the owner said "Do whatever you feel is best".
- Requirement only; no firmware provenance waiver, delegation, live OneNote
  instruction, historical identity or completed edition. Existing scope, policies
  and Macintosh/mini vMac goals preserved.
- SPEC-0009 reuses SRC-0034's reviewed UEFI 12.9.2 GOP pixel-format and
  pixels-per-scanline declarations. On 2026-10-08 the lead tried to re-open the
  publisher PDF (agent proxy 403) and the HTML chapter (fetched text truncated
  before 12.9). No newer UEFI text was reviewed. A web search listed third-party
  GOP header titles, which were not opened or used.

## SRC-0042 — Owner's independent PC-boot continuation

- Owner, 2026-10-08: "ok, let's do it" following the status explanation of the
  next minimal independent PC boot and remaining Macintosh goals. Authorizes
  missing startup components, original specifications/implementation/tests and
  normal Git evidence/wiki plans. No firmware provenance waiver or boot claim.

## SRC-0043 — Intel published x64 descriptor and exception interfaces

- Intel SDM Volume 3A, 253668-090US, February 2026, accessed 2026-10-08;
  [publisher PDF](https://cdrdv2-public.intel.com/874249/253668-090-sdm-vol-3a.pdf),
  SHA-256 `42166ab4aeb53df119a794a1da36eaa8a9f123eea61d90e8fe6195da4ef6654f`.
  Sections 3.4–3.5 (segment descriptors/tables), 6.8.5.1 (far transfers),
  7.12.1.3–7.14.5 (interrupt gates/error codes/frame/IST), Table 7-1 / PDF 202–203 and 7.15
  (exception vectors), 10.2.3–10.2.4 and 10.7 (TSS descriptor/register/format).
  Figures 7-8, 10-4, 10-11 visually reviewed at PDF pages 220,295,308;
  associated prose PDF 217–222,294–295,307–308. Public ISA layouts/prose only.
- Extends SRC-0039 Volume 2A 253666-093US, September 2026, same pinned hash;
  LGDT/LIDT printed 3-553–555 / PDF 671–673 reviewed. No sample implementation,
  OS source, external headers or firmware adopted. Independently specified table
  layout, allocation and terminal policy; no historical Macintosh applicability.
- Initial sandbox download lacked DNS and local PyMuPDF was absent; reviewed
  publisher download succeeded with network permission, using bundled pypdf and
  PDFium rendering. Manual remains local, linked rather than vendored.
- TEST-0023 addendum (2026-10-08, Claude): §11.7.13.4 External Signal
  Compatibility, printed 11-33 / PDF 341 of the same volume and hash: A20M#
  masks physical-address bit 20 for external bus memory accesses; not used by
  modern operating systems; may be absent on newer Intel 64 processors. No
  IA-32e exemption is stated there. Used only to decide what to measure.

## SRC-0037 — Owner's native loader and firmware-review continuation

- Owner, 2026-10-08: "OK, do it" after the proposed exact firmware eligibility
  review and native loader. Original interfaces/loader/transition/tests, inspected
  own EFI packaging, evidence/publication and Git wiki plans are authorized.
- Requirement only; no firmware provenance waiver, VM launch, historical identity,
  completed OS edition or OneNote publication instruction.

## SRC-0038 — UEFI Forum published loader interfaces

- *UEFI Specification* 2.11, cover Nov 21, 2024; accessed 2026-10-08.
  [Publisher PDF](https://uefi.org/sites/default/files/resources/UEFI_Spec_Final_2.11.pdf),
  SHA-256 `a64b8e442004b91becc3de9afaf8ca61b259a9a3b436accb6b3711ab5400cee9`.
- Extends SRC-0034's x64 calling/map/GOP/exit review. Sections 4.1–4.4 (printed
  87–94 / PDF 171–178), 7.2.1–7.2.2 (154–156 / PDF 238–240), 7.3.7 and
  7.3.9 (172–173,175–178 / PDF 256–257,259–262), 7.3.16 (193–194 / PDF
  277–278), 7.5.1 (205–206 / PDF 289–290), 7.5.3 (209–210 / PDF 293–294),
  9.1 (255–257 / PDF 339–341), 3.5.1.1/Table 3.4 (85 / PDF 169), Appendix
  D status codes (1984–1985 / PDF 2068–2069): table/CRC, allocation, protocol
  access, watchdog, LoadedImage and removable x64 payload name.
- Eligible public interface prose/declarations; adjacent protocol implementation
  examples were visible, neither copied nor adopted. No external UEFI headers,
  firmware library, example loader or document redistribution. Original bounded
  SPEC-0007 policy is narrower than general UEFI support. No Macintosh applicability.
- SPEC-0016 addendum (2026-10-09, Claude, same hash): §4.3 (printed 90 / PDF 174)
  states FirmwareVendor and FirmwareRevision stay valid after exit; §4.3.1 defines
  them. Table 7.10 *Memory Type Usage after ExitBootServices()* and its note
  (printed 153 / PDF 237): after exit the image owns unused LoaderCode/Data,
  BootServicesCode/Data and ConventionalMemory. Used to bound the read-only
  bit-20 partner to data and free types.

## SRC-0039 — Intel public x64 instruction interfaces

- Intel, *64 and IA-32 Architectures Software Developer's Manual*, Volume 2A,
  253666-093US, September 2026; accessed 2026-10-08.
  [Publisher index](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html),
  [publisher PDF](https://cdrdv2-public.intel.com/929353/253666-093-sdm-vol-2a.pdf),
  SHA-256 `87c5acb6f27e24d9d364841a0d2346c91a8482e36f403409954e2d2a8c817bac`.
- CLD printed 3-141 / PDF 259, CLI 3-148–149 / PDF 266–267, HLT 3-439 /
  PDF 557: direction flag, CPL/interrupt-mask semantics and halt/resume behavior.
- Eligible ISA interface, including instruction pseudocode, not an OS algorithm.
  Original minimal stack/call/terminal assembly uses the reviewed x64 UEFI ABI.
  CLI does not mask exceptions/NMI; descriptor tables/fault handlers are not owned
  in this scaffold. No Intel implementation or firmware adopted. Initial local
  PDF printing failed character encoding, then bounded UTF-8 extraction succeeded.
- SPEC-0014 addendum (2026-10-08, Claude): JMP rel8 `EB cb` (printed 3-503–504 /
  PDF 622–623 of the same volume and hash): short jump with a sign-extended 8-bit
  displacement added to the address of the next instruction. Used with CLI/HLT
  only for the four-byte legacy stub in the FAT32 boot sector; UEFI never runs it.

## SRC-0040 — Exact VirtualBox x64 producer and selected module metadata

- Oracle official v7.2.16 tag `4cf0b89546257f5044534a7b94cde2dac1e8c175`,
  accessed 2026-10-08. Extends SRC-0035; eligible build/license declarations only.
  Recursive source-tree path inventory SHA `d84e06dbfafbe9d8001be6179a662f9d857a1645`
  was used to locate build files, not read implementation bodies.
- [Firmware build manifest](https://github.com/VirtualBox/virtualbox/blob/4cf0b89546257f5044534a7b94cde2dac1e8c175/src/VBox/Devices/EFI/Firmware/Makefile.kmk),
  blob `4910daa4af034774c3408814dfb23dfa9bba92e5`, line 120 and x64 build
  lines 424–453: OVMF.fd is renamed VBoxEFI-amd64.fd; OvmfPkgX64.dsc/fdf are
  selected with VBOX=1/VBOX_WITH_OVMF=1. Conditional feature definitions visible.
- [x64 DSC](https://github.com/VirtualBox/virtualbox/blob/4cf0b89546257f5044534a7b94cde2dac1e8c175/src/VBox/Devices/EFI/Firmware/OvmfPkg/OvmfPkgX64.dsc),
  blob `9cb47d072f0b54c41232785c051e600f41ce62e0`, copyright/license/build
  declarations; [x64 FDF](https://github.com/VirtualBox/virtualbox/blob/4cf0b89546257f5044534a7b94cde2dac1e8c175/src/VBox/Devices/EFI/Firmware/OvmfPkg/OvmfPkgX64.fdf),
  blob `1bee9af6836349d7a67c42663048aac9582a5d82`, VBOX-selected lines 327–334
  include VBoxHfs, VBoxAppleSim and VBoxApfsJmpStartDxe among other modules.
- Their .inf build/license declarations only were read: VBoxHfs blob
  `1894a70523fb781549ae161264204eb898faa123`, VBoxAppleSim
  `30a0fd54471b381df987c2691ad700ef7ec9b428`, VBoxApfsJmpStartDxe
  `6bb7302ceecd63649df9cc2d728f8d7601490ccb`. They declare Oracle copyrights
  and GPL-3.0-only OR CDDL-1.0; implementation filenames were visible, no bodies
  fetched. Names/notices do not prove Apple code inclusion or clean provenance.
- Exact x64 producer is now identified. Installed binary-to-source correspondence,
  transitive source/notice/derivation closure and rights remain needs-review.
  The earlier Bhyve notice is not automatically selected x64 content. No firmware
  extraction, machine-code inspection, uncertain implementation adoption or VM run.
  No external metadata contents are vendored or used to implement the loader.

## SRC-0041 — Microsoft public PE relocation and section interfaces

- Microsoft Learn, [PE Format](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format),
  accessed 2026-10-08; extends SRC-0032. Optional Header Windows-Specific Fields,
  Data Directories, Section Table/Flags and .reloc/Base Relocation Block/Types:
  EFI application subsystem, section/raw/image alignment and permissions, bounded
  directory/block/type-offset layout and DIR64 delta semantics.
- Eligible public file-format prose/declarations. Adjacent import/exception/TLS
  declarations/example visible, unused. Original PowerShell bounded file/map
  auditor; no external PE parser or firmware relocator adopted. This checks our
  unloaded output, not actual firmware loading/relocation or hardware permissions.

## SRC-0033 — Owner's native preboot-contract continuation

- Owner, 2026-10-08: "OK, move on" after proposed firmware handoff/device contracts
  before the first VirtualBox boot. Scoped original contracts/validators/tests,
  provenance and normal evidence/Git wiki publication. No live OneNote request.
- Requirement only; no firmware eligibility waiver, OS boot or historical result.

## SRC-0034 — UEFI Forum published x64 boot interfaces

- *UEFI Specification*, Release 2.11, cover Nov 21, 2024; accessed 2026-10-08.
  [Publisher PDF](https://uefi.org/sites/default/files/resources/UEFI_Spec_Final_2.11.pdf),
  SHA-256 `a64b8e442004b91becc3de9afaf8ca61b259a9a3b436accb6b3711ab5400cee9`.
- 2.3.4 (printed 28–31 / PDF 112–115), 7.2.3 (156–160 / PDF 240–244),
  7.4.6 (203–204 / PDF 287–288), 12.9.2 (445–449 / PDF 529–533): calling
  convention, map stride/key/descriptors, exit lifetime/retry and GOP fields.
- Eligible interface prose/declarations, not firmware implementation. Adjacent
  allocation/exit prose and GOP bitmask sample were visible, not copied/adopted.
  Original fixed-format validators only. HTML 403/oversized web PDF preceded
  publisher download and bounded local extraction. No PDF/example vendored.
- Project limits/policies are distinct from UEFI conformance; no Macintosh claim.
- SPEC-0016 addendum (2026-10-09, Claude, same hash): within 7.2.3, the memory
  attribute definitions EFI_MEMORY_WB 0x8 and EFI_MEMORY_RUNTIME bit 63 (printed
  158 / PDF 242); 2.3.4's identity mapping of map-described memory (printed 28 /
  PDF 112) is why a partner address is accessed through the identity window.

## SRC-0035 — Oracle VirtualBox interfaces and firmware eligibility metadata

- Installed Oracle VirtualBox 7.2.16r174877; read-only version/filename/hash
  inventory, no machine-code inspection. *User Guide for Release 7.2*,
  [publisher manual](https://download.virtualbox.org/virtualbox/7.2.16/UserManual.pdf),
  sections 5.8/5.13/5.14/5.18/5.22/15.35/18.2.3.12, PDF pages 68–69,71–75,
  79–83,411,518–520
  and published device/setting prose. Local manual SHA-256
  `bc95af3c86f226c253070e8c5916057822fc6f97c8beb89b1127797899e8a1ad`.
- VBoxManage.exe SHA-256 `7fda8e54157eb94a5ebd9cea079a9fb8c23ade92ae71e13399dc3431c021a228`;
  VBoxDD2.dll `6cd01002214a130353b9e12405fa836b5efef7461e04e451460e529109796487`.
  The DLL fingerprint is not a standalone firmware-image fingerprint.
- Official [v7.2.16 tag](https://github.com/VirtualBox/virtualbox/tree/4cf0b89546257f5044534a7b94cde2dac1e8c175)
  and path inventory only. [VBoxPkg.dsc](https://github.com/VirtualBox/virtualbox/blob/4cf0b89546257f5044534a7b94cde2dac1e8c175/src/VBox/Devices/EFI/Firmware/VBoxPkg/VBoxPkg.dsc)
  (Git blob 3b663df43a995e45b6b0b9ed79b2b8461a5f28ea) declares ARM/AARCH64,
  so cannot validate x64. [Device packaging manifest](https://github.com/VirtualBox/virtualbox/blob/4cf0b89546257f5044534a7b94cde2dac1e8c175/src/VBox/Devices/Makefile.kmk)
  (blob 146f5548bfb41200031c51cac15fc1168c5bdd36) names embedded amd64 firmware
  inputs. No module implementation, firmware extraction or whole source archive fetched.
- Eligibility: interface/version/build/license metadata only. License inventory
  lists Apple-attributed portions in OVMF/Bhyve; it does not prove Apple ROM code
  or selected-image inclusion. Exact x64 source/dependency/binary closure remains
  unresolved; firmware needs-review and is not adopted. Product/permissive-license
  labels are not approval. External VM/runtime distribution rights remain separate;
  no Extension Pack or guest image adopted. Accessed 2026-10-08.
- VBoxManage modifyvm help failed COM E_ACCESSDENIED; no VM/settings examined or
  changed. Missing guessed metadata URL and ARM manifest trial are unsuccessful
  eligibility leads. Unrelated search snippets discarded; no third-party/Apple-derived
  implementation input. Public manual examples visible, not copied/adopted.

## SRC-0036 — UEFI Forum published ACPI PM timer interface

- *ACPI Specification*, Release 6.6, May 13, 2025; accessed 2026-10-08.
  [Publisher PDF](https://uefi.org/sites/default/files/resources/ACPI_Spec_6.6.pdf),
  SHA-256 `8c7542dd4de974ae47bba71bb0336637fe1e3838daad7692370ab4cf218efd35`.
- 4.8.1.4, 4.8.2.1 and 4.8.3.3 / Table 4.14 (printed 67–68,83; PDF 138–139,154):
  optional counter width/frequency/access. Nearby fixed-register prose was visible,
  unused; 5.2.9 FADT located but not reviewed as parser input.
- Eligible interface for proposed timer contract; no timer/FADT parser or native
  driver implemented, no physical precision claim. HTML 403/incorrect PDF-name
  lead preceded publisher download. No code copied or document vendored.

## SRC-0030 — Owner's standalone-link and second-compiler continuation

- Author/date: owner, 2026-10-08, "Do it" after proposed standalone linking and
  another compiler before native boot work.
- Scope: current-core link/runtime-boundary proof, necessary portable compiler,
  tests/provenance/publication and Git wiki plans. B1 is already passed.
- Requirement only; no firmware/VM selection, native boot or live OneNote request.

## SRC-0031 — Portable GCC development distribution

- Christopher Wellons / w64devkit, [immutable v2.10.0 release](https://github.com/skeeto/w64devkit/releases/tag/v2.10.0),
  published 2026-09-14; release API asset digest/size and
  [pinned README](https://github.com/skeeto/w64devkit/blob/v2.10.0/README.md),
  Usage, Special linking considerations, Notes and Licenses; accessed 2026-10-08.
- Package includes GCC 16.2.0 / GNU Binutils 2.47.20260726. Lock records official
  package and observed compiler/linker hashes. Extracted tool/version outputs and
  bundled COPYING.MinGW-w64-runtime.txt notices were inspected. Upstream
  [UNLICENSE](https://github.com/skeeto/w64devkit/blob/v2.10.0/UNLICENSE) describes
  the packaging project, not every bundled component's license.
- Eligible external development tools; no tool implementation source fetched,
  adopted or copied. README shell examples and runtime-helper descriptions were
  visible, not adopted. GCC/MinGW hosted-test runtimes remain development-only;
  standalone fixtures link no runtime library or helper. No binary redistribution.
- GCC Runtime Library Exception link discovery timed out; no legal conclusion or
  future product-runtime approval relies on its unread text. Preserve bundled
  notices and independently review any later distribution or runtime adoption.

## SRC-0032 — Compiler/linker and PE interface contracts

- Accessed 2026-10-08: GNU [GCC link options](https://gcc.gnu.org/onlinedocs/gcc-16.2.0/gcc/Link-Options.html),
  -nostdlib/-nodefaultlibs/-e; [C dialect options](https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html),
  freestanding; GNU Binutils [ld options](https://sourceware.org/binutils/docs/ld/Options.html)
  and [WIN32](https://sourceware.org/binutils/docs/ld/WIN32.html), entry/map/subsystem;
  LLVM [Clang manual](https://clang.llvm.org/docs/UsersManual.html) and
  [LLD Windows support](https://lld.llvm.org/windows_support.html); Microsoft
  [PE format](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format),
  COFF/optional/section headers and data-directory interface fields.
- Current documentation versions may differ from pinned tools; the observed
  commands/maps/bytes define tested scope. Pinned GCC C-dialect URL was unavailable;
  the public current manual was read. Bundled tool --version/help and header
  declarations are interfaces; no compiler/linker implementation code inspected.
- Examples/interface snippets were visible; original probe/auditor/scripts only.
  No Apple implementation input or historical Macintosh applicability.

## SRC-0028 — Owner's B1 audit and live viewer validation

- Author/date: owner, 2026-10-07–08, "OK, do it" after proposed remaining
  hosted-start audit; on 2026-10-08 selected "I can test now—open the viewer".
- Scope: existing B1/M0.1 audit, necessary combined checks/operator validation,
  ordinary provenance/publication and Git wiki plans. No new core requirement,
  OS boot claim, historical-source approval or live OneNote publication request.
- Owner-operated test is project validation, not a reference-system observation.

## SRC-0029 — Microsoft queued-message interfaces

- Microsoft Learn, accessed 2026-10-08, parameters/returns/remarks of
  [PostMessageW](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postmessagew)
  (updated 2023-03-21) and
  [GetMessageW](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessagew).
- Host message posting, retrieval, quit/error handling and ordering prose.
  Adjacent public examples were visible, not copied or used as implementation.
  Original bounded integration uses existing project Win32 interfaces.
- No input-origin guarantee, historical Macintosh behavior or new runtime/tool
  adoption follows. Posted key messages remain synthetic host-level fixtures.

## SRC-0025 — Owner's debugger checkpoint authorization

- Author/date: owner, 2026-10-07, "Do it" after proposed debugger verification.
- Scope: hosted debugger tests, necessary viewer launch fixes, evidence/publication
  and Git wiki plans. Existing independent core specifications remain authoritative.
- Requirement only; no B1, native boot, historical behavior or live OneNote request.

## SRC-0026 — LLVM LLDB command interfaces and diagnostic lead

- LLVM project, accessed 2026-10-07: [Tutorial](https://lldb.llvm.org/use/tutorial.html)
  command structure, breakpoints, frame variables and thread stepping;
  [command map](https://lldb.llvm.org/use/map.html), source-map and launch workflow.
  Bundled LLDB --help, process-handle help and version output were also inspected.
- Published command-interface prose/examples, not tool implementation source.
  Original verification commands and evidence checks; no copied implementation.
- Diagnostic lead: [LLVM issue 58065](https://github.com/llvm/llvm-project/issues/58065)
  reports a similar WOW64 frame symptom. Its diagnostic assembly excerpt was visible,
  not copied or used as an implementation input. The local failure is independently
  observed; shared root cause is an inference, not established by the report.
- No Apple implementation inputs, historical applicability or universal debugger claim.

## SRC-0027 — Microsoft runtime argument interface

- Microsoft Learn, [__argc / __argv / __wargv](https://learn.microsoft.com/en-us/cpp/c-runtime-library/argc-argv-wargv?view=msvc-170),
  remarks and header requirements, accessed 2026-10-07. Existing pinned stdlib.h
  declarations were checked; runtime implementation source was not inspected.
- Host-only parsed argument count/strings replace raw WinMain string comparison.
  This uses the already linked Windows development runtime, with no new dependency
  or portable-core change. No documentation sample implementation adopted.

## SRC-0023 — Owner's clock continuation and VirtualBox availability

- Author/date: owner, 2026-10-07, after discussing the next clock milestone.
- Instruction: retain installed VirtualBox for future boot tests and move on.
- Scope: SPEC-0005 time/provider/viewer tests and ordinary provenance/publication.
  VirtualBox availability is a project requirement/tooling hint, not an OS boot,
  VM/firmware provenance approval or replacement for Macintosh validation.
- Read-only local VBoxManage version check returned 7.2.16r174877. No VM created,
  altered or started, firmware adopted, package redistributed or existing VM read.

## SRC-0024 — Microsoft elapsed-time and timer interfaces

- Microsoft Learn, accessed 2026-10-07. Parameters/returns/remarks of
  [QueryPerformanceCounter](https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter),
  [QueryPerformanceFrequency](https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency),
  [SetTimer](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-settimer)
  and [KillTimer](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-killtimer),
  plus interface guidance in [Acquiring high-resolution time stamps](https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps).
- Published host-interface prose only. QPC's adjacent C++ example was visible
  during access; no sample implementation copied/adopted. Original portable time,
  host integer conversion, provider transitions, timed strip and independent tests.
- No historical Macintosh applicability, hardware timing certification, UTC,
  nanosecond accuracy or native timer/scheduler implementation claim.

## SRC-0021 — Owner's next implementation step

- Author/date: project owner, 2026-10-07: move to the next step after arenas.
- Requirement authority for SPEC-0004 normalized keyboard input, verification,
  provenance and ordinary publication/wiki maintenance. No historical/boot claim.

## SRC-0022 — Microsoft keyboard message interfaces

- Publisher: Microsoft Learn; accessed 2026-10-07. Interface prose: parameters,
  bit tables, returns and remarks of [WM_KEYDOWN](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keydown)
  and [WM_KEYUP](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keyup),
  plus Space/Escape entries in [Virtual-Key Codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes).
- Scope: Windows adapter only; one delivered message produces one normalized
  event. No Windows sample implementation copied; original mapping/queue/tests.
  The WM_KEYDOWN page's adjacent example was visible during documentation access;
  it was not used as an implementation source. No Apple material involved.
- No Macintosh applicability, keyboard layout/text, hardware driver or clock claim.

## SRC-0019 — Owner's boot-goal reaffirmation and continuation

- Author/date: project owner, 2026-10-07, in the implementation chat.
- Requirement: retain the clean-room, independently implemented 0.97 goal,
  bootable on a real Macintosh or mini vMac; then continue the project.
- Scope: next roadmap contract SPEC-0003 arenas and viewer integration, with the
  established verification/provenance/publication/wiki workflow. mini vMac is an
  intended validation path; existing physical-hardware edition gates remain.
- No emulator/firmware dependency, historical identity or completed boot approved
  by this requirement. Inputs remain behavioral contracts, never Apple code.

## SRC-0020 — WG14 C11 committee draft interface semantics

- Publisher: ISO/IEC JTC1/SC22/WG14, N1570, 2011-04-12; accessed 2026-10-07.
- [Committee draft](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf),
  sections 6.2.8 (alignment), 6.5 paragraphs 6–7 (effective type),
  6.5.6 (bounded pointer arithmetic), 7.19 (max_align_t) and 7.22.3
  (host allocation alignment/lifetime).
- Eligibility: published language/interface prose only. Independently designed
  monotonic arena and tests; no sample implementation copied or adopted.
- No historical Macintosh applicability, allocator algorithm source, runtime
  dependency or general compiler certification. Host heap use stays in the adapter.

## SRC-0017 — Owner's next milestone authorization

- Author/date: project owner, 2026-10-07, after the surface handoff.
- Instruction: proceed with the proposed Windows presentation/original-scene
  checkpoint, using the established implementation, verification, publication and
  wiki workflow. Requirement authority only; no historical or boot certification.

## SRC-0018 — Microsoft Win32 presentation interfaces

- Publisher: Microsoft Learn, live public Win32 documentation, accessed 2026-10-07.
- Sections used: parameters, return values and remarks of
  [StretchDIBits](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-stretchdibits),
  [BITMAPINFOHEADER](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader),
  [BITMAPINFO](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfo),
  [CreateDIBSection](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createdibsection),
  [RectVisible](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-rectvisible),
  [IntersectClipRect](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-intersectcliprect),
  [PrintWindow](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-printwindow)
  and the [painting lifecycle](https://learn.microsoft.com/en-us/windows/win32/learnwin32/painting-the-window).
- DPI: parameters/returns/remarks of
  [SetProcessDpiAwarenessContext](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setprocessdpiawarenesscontext),
  [AdjustWindowRectExForDpi](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-adjustwindowrectexfordpi),
  and message semantics of [WM_DPICHANGED](https://learn.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged).
- Eligibility: host interface prose and existing pinned Windows header signatures.
  Independent SPEC-0002/code; no documentation sample implementation, third-party
  rendering code or artwork incorporated. No Macintosh historical applicability.
- Limits: Microsoft recommends manifest DPI configuration for production apps.
  This bounded development viewer uses the documented API before any UI, avoiding
  new resource tooling. Actual cross-monitor changes/physical colors unverified.

## SRC-0016 — Owner's first subsystem implementation assignment

- Author/date: project owner, 2026-10-07; instruction retained by the owner.
- Public derivative: SPEC-0001 v1 and IMPL-0002, independently summarized.
- Classification: project requirement/implementation authorization after bootstrap.
- Scope: finalize bounded surfaces, implement without heap/platform dependencies,
  verify independent headless tests, freestanding boundaries and supported
  sanitizers, maintain provenance/status/wiki, commit and push the scoped result.
- Eligibility: requirement authority only, not a historical behavioral source.
- Limits: surfaces do not establish M0.1, QuickDraw, any bootable edition or exact
  January 1984 historical identity. No third-party implementation/asset input.

## SRC-0001 — Owner's founding brief

- Author/date: project owner, 2026-10-07; private instruction retained by owner.
- Public derivative: [charter](../docs/charter.md), independently summarized scope.
- Classification: authorized project requirements and design constraints.
- Applicability: all project decisions; not primary historical evidence.
- Limitations: internal System 0.97 identification and exact reference versions
  remain research questions; the brief is not proof of Apple implementation details.

## SRC-0002 — LLVM-MinGW publisher documentation and release

- Author: Martin Storsjo / LLVM-MinGW maintainers.
- [Project documentation](https://github.com/mstorsjo/llvm-mingw);
  [release 20260908](https://github.com/mstorsjo/llvm-mingw/releases/tag/20260908).
- Sections used: Releases / UCRT package descriptions; release asset metadata.
- Eligibility: development-tool documentation and binaries only, not historical
  implementation input. No upstream toolchain source copied into project code.
- Evidence: package URL/version/size/SHA-256 pinned in `toolchain-lock.json`;
  setup and native compiler output were independently exercised locally.
- Limits: Windows hosted support does not prove original m68k or bare-metal support;
  documented debugger/sanitizer capability is not verified local workflow evidence.

## SRC-0003 — Kitware CMake/CTest documentation and release

- [CMake 4.4 presets manual](https://cmake.org/cmake/help/v4.4/manual/cmake-presets.7.html),
  Introduction and configure/build/test preset fields; publisher: Kitware.
- [Release 4.4.4](https://github.com/Kitware/CMake/releases/tag/v4.4.4), asset metadata.
- Eligibility/scope: build-system interfaces and pinned tooling, not compatibility
  semantics. Used to author independent project presets/scripts.
- Limitations: bootstrap tests only the configured native Windows flow.

## SRC-0004 — Ninja publisher

- [Ninja project](https://ninja-build.org/);
  [release 1.13.2](https://github.com/ninja-build/ninja/releases/tag/v1.13.2).
- Scope: CMake-generated build execution and release asset pinning.
- Eligibility: developer infrastructure; no source incorporated.

## SRC-0005 — UEFI Forum specification

- Publisher: UEFI Forum, UEFI Specification 2.11.
- [Official specification](https://uefi.org/specs/UEFI/2.11/), boot/runtime services
  lifecycle and Graphics Output Protocol; [official PDF](https://uefi.org/sites/default/files/resources/UEFI_Spec_Final_2.11.pdf).
- Scope: planned firmware handoff contract; no native implementation yet.
- Eligibility: public interface specification, linked rather than republished.
- Limits: selecting the specification is not tested firmware support; exact entry,
  memory-map, ExitBootServices and GOP sections must be reviewed before coding.

## SRC-0006 — GCC M680x0 compiler options

- Publisher: GNU GCC project, current compiler manual accessed 2026-10-07.
- [M680x0 Options](https://gcc.gnu.org/onlinedocs/gcc/M680x0-Options.html),
  `-m68000` and `-mshort` descriptions.
- Scope: candidate native 68000 compiler direction and ABI warning.
- Eligibility: tool documentation; no Apple implementation content.
- Limits: no Windows-hosted m68k package has been selected or executed. Option
  documentation does not prove complete freestanding runtime/link support.

## SRC-0007 — Apple archived QuickDraw documentation, candidate only

- Publisher: Apple, *Inside Macintosh: Imaging With QuickDraw*, archived web index.
- [Imaging With QuickDraw](https://developer.apple.com/library/archive/documentation/mac/QuickDraw/QuickDraw-2.html).
- Inspected: overview/index, for legitimate public-documentation discovery only.
- Eligibility: published prose/API documentation candidate; no sample code copied.
- Applicability: later Macintosh documentation; **not approved as a 1984 behavioral
  oracle**. Establish edition/date and exact sections before any derived spec.
- Limits: no QuickDraw behavioral implementation or historical fact is based on it.

## License/reference-policy sources

| ID | Publisher and exact source | Sections / purpose |
| --- | --- | --- |
| SRC-0008 | [Apache Software Foundation, Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0) | Patent grant/termination and distribution conditions; decision comparison only |
| SRC-0009 | [Open Source Initiative, BSD-2-Clause](https://opensource.org/license/bsd-2-clause) | Permissive terms and retained notices; decision comparison only |
| SRC-0010 | [Free Software Foundation, GPL v3](https://www.gnu.org/licenses/gpl-3.0.html) | Corresponding Source, distribution, patents and installation obligations; selected after owner approval |
| SRC-0011 | [Mozilla, MPL 2.0](https://www.mozilla.org/MPL/2.0/) | Covered files, distribution and patent terms; comparison only |
| SRC-0012 | [GitHub, Licensing a repository](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/licensing-a-repository) | Public visibility, absent-license rights and GitHub view/fork exception |
| SRC-0015 | [Open Source Initiative, MIT](https://opensource.org/license/mit) | Permissive use and notice retention; comparison only |

The owner approved GPL-3.0-or-later on 2026-10-07. The standard GPL v3 text in
`LICENSE` was retrieved verbatim from the [FSF publisher](https://www.gnu.org/licenses/gpl-3.0.txt)
after that approval. Download SHA-256:
`3972dc9744f6499f0f9b2dbf76696f2ae7ad8af9b23dde66d6af86c9dfb36986`.
The project's version-3-or-later grant is stated explicitly in README/COPYRIGHT.md
and source SPDX notices; the standard license text itself is not modified. Other
listed license texts are alternatives, not adopted terms. No compatibility inference
follows from any license.

## SRC-0013 — GitHub Actions security guidance

- [Secure use reference](https://docs.github.com/en/actions/reference/security/secure-use).
- Scope: full-commit action pinning, least-privilege workflow design.
- Eligibility: development/security documentation; no source copied.

## SRC-0014 — Official checkout action release

- Publisher: GitHub / `actions/checkout`.
- [Pinned official revision](https://github.com/actions/checkout/tree/3d3c42e5aac5ba805825da76410c181273ba90b1),
  tag v7.0.1 verified through the official repository's tag reference API.
- Scope: CI source checkout; configured without credential persistence.
- Code is executed as an external pinned action, not vendored into this project.
- CI execution and hosted runner revisions are recorded separately; runner images
  can change despite tool-package pins.
