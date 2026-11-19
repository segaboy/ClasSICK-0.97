<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0136 — Synchronous read and overwrite on one open MFS fork

Published Inside Macintosh (1985), Volume II, pp. 100, 110–111, and the Volume
III trap summary document synchronous PBRead/PBWrite, parameter-block fields,
position modes and result codes. Source is SRC-0071, [published edition](https://puyoman.net/documentation/Inside_Macintosh.pdf).
Consulted 2026-11-19 for published interface facts only. January 1984
revision coverage remains unverified. The native reference binding, bounds
and refusal rules below are original engineering decisions.

An owner binds one decoded and validated fork from an exact counted MFS name
to an assigned positive reference number, with mark zero and optional write
permission. A successful rebind replaces the previous access path. Its
volume, device, writer and exclusive metadata remain stable as in SPEC-0128.
The caller owns two disjoint scratch areas for transfers and the file record.
The synchronous service accepts canonical A002 and A003, with A0 pointing
to a 50-byte big-endian parameter block in a checked guest map. Native
objects and guest-region backing are live, truthful and physically disjoint,
with each region at most 32767 bytes and confined to one native segment.
Neither PB nor data areas may overlap. No interrupts, callbacks, DMA,
concurrent mutation or reentry during a call.

Fields used are ioResult (16), ioRefNum (24), ioBuffer (32), ioReqCount (36),
ioActCount (40), ioPosMode (44) and ioPosOffset (46). Modes 0–3 select
current mark, start, logical EOF and relative-to-mark. Mode zero ignores the
supplied offset. Successful calls update count, position and the open mark.
A read beyond EOF returns the available prefix and eofErr. Negative request,
bad reference, lock/permission and negative position receive corresponding
OSErr values. OSErr is published to D0 and ioResult. On provider failure a
write reports its acknowledged prefix; its failing sector may have changed.
Staged failing reads publish no data. Unsupported calls, write extension,
unsupported flags, excessively large counts/positions, invalid guest windows
and aliasing are explicit native refusals before disk writes and before guest
output publication. This slice covers existing forks only.

Verify through generated media on both processors, including stale-reference
refusal, negative relative positioning, partial EOF, fragmented overwrite,
whole exported media comparison and fresh protected guest readback.

The Mac probe executes A002/A003 through the processor's line-A vector and
returns with RTE. The native 286 narrow service step fetches the opcode at
the 68k frame's PC from executable guest memory, checks operand access,
and advances PC by two after a handled call. This explicit step uses disjoint
near backing for each guest region; integration with the banked CPU router,
asynchronous calls, PBOpen/PBClose, patching and historical CCR behavior
remain outside this profile. Native OSErr values are sign-extended for D0.
