<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# November 21 System Disk startup trials

The replacement 64 KiB Macintosh ROM returned successfully from NewPtr on
the supplied System Disk. It completed 906 instructions in the two loaded
boot sectors, up from [38 on November 20](startup-service-boundary-20261120.md).
A separately qualified diagnostic identified **PBMountVol** as the next
service the ROM could not supply in three cold starts. The volume mount,
System startup and Finder remain open.

The installed 286 executed the same allocator from generated 68k code.
The supplied System Disk reached the interpreter three times and stopped
during the third dispatch, before entering NewPtr. The last guest address
reported by the [November 20 CPU diagnostic](startup-service-boundary-20261120.md#bounded-cpu-diagnostic)
was `$808E`. The normal installed runtime reports only its native stop.

## Observations on normal builds

The [runner](../../scripts/Test-StartupServiceBoundary.py) installed the
286 runtime on a fresh fixed disk. An independently generated boot sector
called GetTrapAddress and NewPtr, checked the status and pointer, and wrote
and read back a value before reaching its deliberate illegal instruction
on both machines. The installed interpreter made 12 dispatches. The runner
then used the separate supplied floppy in three cold trials per target.
All three observations on each target matched. Source and installed media
identities matched after the runs.

| BootHush observation | Result |
| --- | --- |
| Mac entered loaded startup code | `$8002` at 4,366,882 modeled cycles. The first instruction completed at `$808A`. |
| Mac allocated memory | Common NewPtr entry at 4,370,092 cycles, success at 4,399,580, and service return at 4,399,710. The allocation-failure branch was unreached. |
| Mac continued in the caller | Another Line-A dispatch at 4,407,696 cycles. The ROM reached its common stop at 4,433,840. All 906 instructions started inside the two loaded sectors completed within the 16-million-cycle window. |
| Installed 286 executed the disk | Three visits to the native dispatcher, from 7,449,684 to 7,770,065 cycles. POST `$D2` at 7,882,759 cycles. The NewPtr callback was unreached. |

The [32 generated cold boots](newptr-20261121.md) exercise pointer writes,
application and system zones, clearing across a 286 bank boundary, failure
handling, and patch/restore. The [service-name diagnostic](startup-unhandled-service-20261121.md)
records its separate 12 opt-in and 60 default controls. Only the diagnostic
identifies PBMountVol. The normal build demonstrates allocation, return to
the caller and subsequent dispatch.

| Artifact or receipt | SHA-256 |
| --- | --- |
| Supplied System Disk DC42 | `1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647` |
| BootHush executable | `543767ae09f39d330803c05bd34311b3b3f485e53f00ec814c54c82d9c8a329f` |
| Developer Mac ROM | `5c902902c9b262c74bd05306013577fe8535514f569fa8808b0e33fbe5be58fd` |
| Developer 286 installer | `a182f4326fce3868438fc093eb924775dd0ceb1df8ade29d9a6eb519ab5dd07b` |
| Installed hard disk | `d7e2d071a41c67fee7f23c58219741f2edd56bce2f3da2c0c85f68518ff2bb04` |
| Build manifest | `a40eef65666ea8effa6292821248caf34aeac4c7040846c4c1ebb497b1b0a0dd` |
| Generated qualification | `f434d4842f86f94be4e74ff945b434025f5b7d96cd44692a7726c606e6fd6655` |
| Normal selected-disk trial | `d1dc6e988b215a3a2f973f4792cd11926dd8de1b655fb9bb82889dd1841c2e00` |
