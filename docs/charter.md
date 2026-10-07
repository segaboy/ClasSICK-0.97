# Project charter

Status: approved founding requirements; dated 2026-10-07.

## Mission

Create an independently implemented, portable OS capable of reproducing useful
documented and externally observable behavior of the January 1984 Macintosh
System 1.0/System 0.97 environment. The public project name is **ClasSICK 0.97**;
the GitHub slug is `ClasSICK-0.97`.

## Non-negotiable constraints

1. Zero original Apple executable or ROM code and zero prohibited derived code.
2. Public development with auditable specifications, provenance, and tests.
3. Native OS builds beyond Macintosh hardware; CPU translation is confined to
   legacy application instructions on non-68000 architectures.
4. A portable core that does not assume guest byte order, pointer width, RAM size,
   display geometry, monochrome pixels, MFS, floppy storage, or Macintosh input.
5. Independently created graphical assets. Historical behavior and extensions
   have separate profiles, claims, and tests.
6. Reproducible Windows 11 development starting from Windows and Git; extra tools
   have documented purposes, pinned versions, and setup procedures.
7. The owner selects licensing. Public publication does not itself make this an
   open-source project. The owner approved GPL-3.0-or-later on 2026-10-07 for
   project-owned code, documentation and assets; third-party rights stay separate.

## Scope and success

The owner reaffirmed the clean-room Macintosh boot goal on 2026-10-07 and named
mini vMac as an intended validation path (SRC-0019). Our native Mac OS/replacement
firmware image must be tested in the selected emulator and on declared original
hardware. Emulator version/configuration, startup assumptions and independent
firmware support require review; no emulator boot or tool adoption is claimed.
This adds a validation path, not a fourth edition or replacement for physical gates.

Bootstrap succeeds when the public repository contains binding safeguards,
architecture/ADRs, source catalog, target/toolchain strategy, measurable milestone
gates, an honest environment inventory, and verified repeatable build probes.
It does not include OS subsystem implementation or a working Macintosh environment.

Hosted-core success requires the same freestanding core library to drive an
abstract surface and events under Windows, with memory/input boundary tests.
Native-boot success is defined separately in [boot gates](architecture/boot.md).

Every completed 0.97 version requires three independently tracked bootable editions:
original Macintosh hardware with independent replacement firmware where required,
native PC hardware without Linux as its base, and Linux-based PC hardware. The
first two target functional parity against the same System 1 contracts. Linux may
evolve into modular ClasSICK through explicit versioned divergence. A hosted
process or one successful edition cannot establish version completion. All three
physical-hardware gates remain pending; VM tests are earlier development gates.

Version 1.0 additionally requires native boot on x86-64 and Motorola 68000, an ARM64 hosted or
native build exercising the same core, documented System 1 API coverage, MFS and
fork interoperability tests, and a named application corpus executing against our
Toolbox implementation. Each included API/app has profile-specific tests and
documented limitations. A compatibility report identifies supported operations,
error paths, deviations, and gaps; an empty or selectively cherry-picked corpus
does not satisfy the gate. Freeze the release corpus and minimum coverage before
claiming 1.0. No universal application-compatibility claim is authorized.

Macintosh 128K support remains required but feasibility is unproven. If memory,
ROM, timing, or hardware constraints require a reduced configuration, document it
and obtain an explicit scope decision rather than silently dropping the target.

## Governance and change control

The umbrella ClasSICK HQ owns cross-project architecture, governance and major
scope decisions. The owner is final authority on licensing and release scope.
The 0.97 main project chat owns day-to-day specifications, implementation, tests,
provenance and progress. Its first surface implementation assignment was explicitly
authorized on 2026-10-07. Maintainers review provenance; no independent human review
is implied by an agent's self-review.

Architecture decisions use numbered ADRs. Specifications use stable IDs and exact
version applicability. Scope changes update the charter, roadmap, affected ADRs,
and test criteria together. Technical debt carries an owner, consequence, and
resolution gate. [Specialist roles](governance.md) are proposals for future work;
no specialist chat is created by bootstrap.

## Non-goals for the first release

Pixel-for-pixel Finder reproduction; copied artwork; full-machine emulation as an
OS portability mechanism; System 6/7 implementation; networking; modern browser
support; a general multiprocessor/security-hardened desktop; unmeasured blanket
claims about all Macintosh software. Architecture may allow future work without
committing this project to it.
