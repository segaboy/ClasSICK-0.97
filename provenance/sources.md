# Source catalog

Access/review date for external sources: **2026-10-07** unless noted. Catalog
eligibility applies to the stated scope, not every resource reachable from a URL.
No Apple executable, source, disassembly, decompilation, or copied asset was used.
No reference-system black-box observation has yet been performed.

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
