# Source catalog

Access/review date for external sources: **2026-10-07** unless noted. Catalog
eligibility applies to the stated scope, not every resource reachable from a URL.
No Apple executable, source, disassembly, decompilation, or copied asset was used.
No reference-system black-box observation has yet been performed.

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
