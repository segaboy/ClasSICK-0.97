<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SRC-0627 - RAM address decoding on the original Macintosh

Apple Computer, *Inside Macintosh*, Volume III (1985), chapter 2, printed
III-17, III-42 and III-45. III-17 assigns the first four MiB of processor
address space to RAM and describes the repetition of addresses within the
hardware regions. III-42 describes the startup overlay. III-45 identifies
shared 128K/512K screen and sound addresses and the reason they work on the
smaller machine. [Published combined edition](https://puyoman.net/documentation/Inside_Macintosh.pdf).

For the 128 KiB guest with its startup overlay released, this implies that
a bus access in the low four MiB selects physical RAM using its low 17 bits.
For instance, a 16-bit access to $03FFF0 addresses the physical word at
$01FFF0. Repeating the address range adds no physical storage. This source
addresses the low RAM mapping. It does not specify the software state or
startup calling convention of the supplied disk.

The lead checked the published hardware descriptions on 2026-11-19.
The triggering observation was a numeric read address in the bounded
[System Disk trial](../../docs/development/boot-handoff-20261119.md).
The mapping is derived from the published hardware facts independently of
the disk. No guest instruction, ROM listing or historical implementation
supplied the addressing rule. Physical hardware qualification remains open.
