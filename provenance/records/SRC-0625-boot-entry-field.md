<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SRC-0625 – Published description of the boot-block entry field

Micro Analyst, Mac Zap V4.54 Manual, 1987, Appendix B “Boot Blocks,”
printed p. 59. Accessed 2026-11-19. The disk-tool publisher's user manual
describes the role of the two startup blocks and identifies the four-byte
field beginning at offset 2 as a branch. Its description supplies the
entry-field hypothesis for SPEC-0133. No utility source, sample instructions,
boot code or screenshots were copied or used. The original verification
programs were authored for ClasSICK.

[Published user manual](https://www.bitsavers.org/pdf/microAnalyst/Micro_Analyst_MacZap_4.54_Manual_1987.pdf)

This manual postdates the original Macintosh release. Its description does
not establish ROM revisions, the load address, live registers, startup return
convention, version-specific choice of startup routine or initial manager
state on a particular January 1984 system. SRC-0622 provides contemporary
corroboration of the initial two-sector read; SRC-0617 records the later
header source and limitations on version-dependent dispatch.
