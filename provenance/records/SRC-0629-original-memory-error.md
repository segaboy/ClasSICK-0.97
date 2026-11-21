<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SRC-0629 - Last Memory Manager error on the original Macintosh

2026-11-21. Apple's final Macintosh Software Supplement Update, dated May
1985 and distributed with the July 2 developer letter, includes a debugger
reference called LoNumeric.Text, dated April 12. Its numeric table identifies
MemErr at $0220 as a two-byte last Memory Manager error. The introduction
cautions developers about undocumented locations. Reviewed only the publication
identity and the interface fact in the $0220 row. No program text, executable
implementation or private media was used. [Published supplement](https://www.vintagecomputer.ca/files/Apple/Macintosh/developer/MacSupplement-May.1985.pdf)

The published 1985 Inside Macintosh, Volume II, pp. II-27, II-36, II-44
and II-48 distinguishes the linked Pascal MemError function from the
NewPtr assembly interface, which returns a result code in D0. Neither the
supplement row nor this language-interface description establishes which code
wrote $0220 on each bare version-105 trap or what it held before the first
call. SPEC-0146 publishing the NewPtr result there is a compatibility
inference. Later descriptions of the 128 KiB ROM are a separate edition.
[Published Volume II](https://vintageapple.org/inside_o/pdf/Inside_Macintosh_Volume_II_1985.pdf)
