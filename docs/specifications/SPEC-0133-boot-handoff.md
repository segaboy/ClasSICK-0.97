<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0133 – Bounded 64K-ROM startup handoff experiment

The Macintosh reads logical sectors 0 and 1 at startup (Apple, published
Inside Macintosh III, 1985, p. 42, SRC-0622). Micro Analyst's published
Mac Zap 4.54 manual (1987), Appendix B, p. 59 (SRC-0625) describes startup transfer
through a branch field at offset 2. Its year and model scope leave the
initial 128K ROM revision and version policy uncertain. The later HFS-era
header descriptions in SRC-0617 do not resolve the old MFS policy.

Exercise a single test hypothesis: after the existing SPEC-0130 header checks
succeed, place the complete two-sector image at $8000 and enter at $8002.
The load base, register seeds, stack, synthetic return boundary and dispatch
decision are engineering choices. A valid header alone does not establish a
historical transfer for a particular version. All D and A registers start
zero except A0 (load base) and A7 ($77FC pointing to a synthetic $7FFE
return address). SR is $2700. SPEC-0137 initializes four documented globals
at their 128K logical addresses. Other ROM services, vectors, globals and
device state remain incomplete. The 286 uses 128 KiB guest RAM and an
explicit stack interval [$7000,$7800]. It executes up to 256 guarded steps
that start inside the 1024-byte image. Stop immediately on a failed step,
outside continuation, return, or the limit.

The Mac runs natively after its independent reset and masked IWM read. The
observer stops before entry, after the first modeled instruction and at a
bounded subsequent cycle horizon. No private code, general registers,
instruction data, stack, framebuffer or disk contents may be exported by
the harness. PC/status counts are diagnostic observations only. Original
generated return, invalid-header, and second-sector fault controls precede
any authorized read-only selected trial. All transport copies are external.
