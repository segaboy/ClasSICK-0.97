<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0173 - Native Macintosh disk-startup trap-address services

2026-11-20. SPEC-0143 / TEST-0234. Original Codex-assisted work in
`arch/m68k/disk-startup-reset.S` and `platform/mac128k/disk-startup.c`
connects vector 10 to the existing SPEC-0020 directory, executes a selected
RAM or ROM routine on the guest's stack, and resumes through RTE. The named
Get and Set entries are callable addresses. The directory occupies the disk
workspace after acquisition. Inputs were the project's published register
and exception contracts and original implementation sources. GPL-3.0-or-later.
Human provenance review remains open.
