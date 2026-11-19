<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
# ADR-0084 - Original ROM boundary and installed 286 equivalent

Accepted 2026-11-19 under owner SRC-0621. Published facts are reviewed in
SRC-0622; [the canonical audit](../development/original-rom-parity-20261119.md)
owns the responsibility register, reuse assignments, four-disk identities and
completion gates R1-R8.

The production Mac component replaces the original 128K Macintosh's 64 KiB
ROM responsibilities. Disk System/Finder, packages, applications and replacement
desktop work remain distinct. A bootable demonstration or one working disk
cannot establish all original ROM services.

The 286 installer deploys a standalone native runtime with equivalent services
and execution of disk-resident 68000 startup, System/Finder, patches/resources,
applications and accessories. The installer and installed runtime are one
delivery obligation; a completed preview installation alone does not satisfy it.
Native managers remain native, while original disk code executes through the
PC compatibility boundary. Preserve the existing installer and all useful code.

This narrows component completion relative to the wider charter and clarifies
application-only execution wording in ADR-0001/ADR-0043 and the architecture;
SRC-0612 already admitted disk-resident system execution. ADR-0040's separate
ROM/System versions remain. No default expanded feature set, substituted desktop,
larger ROM/RAM assumption or whole-machine emulation requirement is introduced.
Old previews and unrelated dirty work stay intact. Generalized core internals
may be shared; production build inclusion must follow the frozen contract.

The alternative of declaring complete-ROM success from a synthetic preview or
selected disk boot is rejected because it leaves unexercised services and
hardware behavior unproved. Absorbing all disk software into ROM would erase
the historical dependency boundary and the independent System product.

This decision changes requirements and ownership, not implementation or evidence.
Routine-level inventory, combined 128K feasibility, disk workloads, physical
qualification and complete runtime installation remain open.
