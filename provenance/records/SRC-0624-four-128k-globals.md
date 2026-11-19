<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SRC-0624 - Four low-memory pointers for the Macintosh 128K profile

Published Apple *Inside Macintosh*, 1985, Volumes II and III, as admitted by
SRC-0071. II-19, figure 9, and II-44 describe RAM organization and the
end-of-RAM and screen pointers. III-45 gives the main screen and sound
buffer locations, the ROM base, and explains how the higher RAM addresses
alias on the 128K machine. Appendix D, III-229 to III-230, locates the four
global variables. [Published combined edition](https://puyoman.net/documentation/Inside_Macintosh.pdf).

SPEC-0137 uses the lower canonical RAM addresses for its 128 KiB profile,
with 32-bit big-endian pointers. The published sources specify meanings and
locations. Populating them before a bounded handoff is an engineering choice.
The exact initialization order and complete register/stack contents at the
original disk's entry remain open.

The 1992 Memory manual, printed 2-14, describes boot-block placement for
128K **ROM** with System 4.1. That passage does not establish placement on a
128K **RAM** Macintosh with the 64K ROM. A general web search also returned
ineligible implementation-derived snippets. Those results were excluded and
no repository was opened. Only the named published Apple volume passages
supply the interface facts above. No third-party code, boot algorithm,
protected disk contents or source example was adopted.
