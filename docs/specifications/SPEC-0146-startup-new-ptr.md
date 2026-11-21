<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0146 - NewPtr on the Macintosh ROM and 286 startup installer

2026-11-21. IMPL-0176 / TEST-0237. The 1985 edition of Inside Macintosh
describes allocation of nonrelocatable blocks in Volume II, pp. II-27, II-36
and II-48. The register trap receives a signed long byte count in D0, returns
a guest pointer in A0 or NIL on exhaustion, and reports noErr (0) or
memFullErr (-108) in the low word of D0. CLEAR zeroes the requested payload.
SYS selects the system zone. The 64 KiB ROM generation's documented
last-error word and the limit of the historical evidence are recorded in
[SRC-0629](../../provenance/records/SRC-0629-original-memory-error.md).
Preserve the input high word of D0 and apply SPEC-0023's CCR rule. A word
test of the result sets N/Z and clears V/C while preserving X.

| Word | Allocation |
| --- | --- |
| A11E | Current zone |
| A31E | Current zone, clear |
| A51E | System zone |
| A71E | System zone, clear |

Both targets consult the installed trap directory before invoking NewPtr.
The real 68000 takes vector 10 and resumes by RTE. The installed 286
interpreter uses its guarded banked dispatch. A replacement RAM routine
runs in place of either native implementation. The Macintosh directory
exposes a callable plain NewPtr ROM address via GetTrapAddress. A direct
JSR to it uses the plain current-zone operation. An ordinary allocation
error returns to the caller. Invalid dispatcher state or an unsupported
routine uses the target's terminal path.

## Startup heap profile

The two targets use the same fixed placement after the disk handoff.
They reserve the region through $009FFF for low memory, ROM-owned data on
Mac, the downward startup stack, and loaded startup code. Two opaque zone
identities address reserved $80-byte bands. System allocations start at
$00A080 and stop before $010000. Application allocations start at $010080
and stop before $01A700, the published screen base on the 128 KiB model.
The addresses and reserved bands describe this implementation's geometry.
The zone identities do not expose historical heap headers. Fixed blocks
remain live until machine restart. Disposal, resize, handles, compaction
and heap growth remain separate service work.

Seed HeapEnd ($114) and ApplLimit ($130) to $01A700, TheZone ($118)
and ApplZone ($2AA) to $010000, SysZone ($2A6) to $00A000, and Lo3Bytes
($31A) to $00FFFFFF. TheZone selects the current zone at each allocation.
An even ApplLimit within the configured application payload range controls
new application allocations, including after the limit drops below a
previous allocation. Neither lowering it nor selecting SYS invalidates an
existing block. Zone extents and identities are implementation constants.
Both cursors are 32-bit even guest addresses maintained by the runtime.

Reserve two bytes for a zero-length allocation. Round other lengths up to
even and clear only the requested bytes for CLEAR. A plain allocation does
not initialize payload bytes. Check remaining capacity before rounding so a
negative or overflowing long request cannot wrap. A failed allocation cannot
advance a cursor or modify an allocation. Guest memory at $0220 receives
the post-call error word as the SRC-0629 compatibility inference. Heap
initialization leaves this word untouched. The later Pascal MemError
interface is outside this native service. Physical-machine and original
system-disk results require their own evidence.
