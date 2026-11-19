<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0130 – Two-block startup image acquisition

SRC-0617, SRC-0622 and SPEC-0111/0119 supply the logical-block and
header interfaces. Both native targets need to acquire a complete image before
transferring control. This component publishes the opaque bytes of LBA 0 and 1
together with their validated SPEC-0119 metadata. It applies the existing
signature and counted-name profile. On any refusal, every output byte remains
unchanged. On success, bytes match disk order, including the full second block.
The caller provides a disjoint device, workspace and properly aligned output.
All live objects fit one native segment, remain stable for the operation and
satisfy the underlying device and storage contracts. No allocation occurs.

The 286 test uses the SPEC-0110 installer on a blank modeled hard disk, checks
the complete output image, and cold boots the installed payload with a separate
read-only transport supplying the 800 Macintosh sectors at LBA 0.
The test harness supplies a project-authored disk with its own entry convention.
It enters at image+2, branches across the header and returns the 32-bit disk
value in D0. The Mac calls the image from native RAM. The PC maps the image
into its banked guest address space and uses the existing guarded CPU engine.
The test entry, stack, register state, return target and image placement belong
to the fixture. Historical startup ABI and dispatch, System/Finder execution and
physical interoperability remain open. The harness accepts only its own generated
media, with no private-disk option.
