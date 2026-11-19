<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0167 - Seed four startup pointers

Original freestanding implementation of SPEC-0137 in
`personality/system1/startup-globals.c`. The caller maps a single native
window onto guest RAM beginning at $0100. Explicit byte stores support both
guest byte order and the native 286's segmented addressing.
