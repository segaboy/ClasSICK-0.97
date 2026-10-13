# SPEC-0023 - Bounded pointer and handle Memory Manager

Finalized v1, 2026-10-12, lead self-review; human provenance review pending.
SRC-0101-0103 / ADR-0027 / IMPL-0027 / TEST-0031.
Engineering profile `classick-memory-manager-v1`; not January1984 confirmation.

## Reviewed interface facts

SRC-0102, owner's published 1985 Inside Macintosh: I-94/95, II-27,
II-30/34, II-36/37, II-41, II-44/46/48/50; Appendix C III-215/220
and globals III-227/231. NewPtr/NewHandle return an even logical payload/handle
or NIL, with CLEAR zeroing and SYS selecting the system zone. Handles identify
a four-byte master cell. Its low24 bits hold the payload address and its upper
byte has lock/purge/resource flags (bits7/6/5). Locked payloads and pointers do
not move; handle identities survive movement. Size queries return logical sizes.
BlockMove supports overlap in the later manual. Register results use D0's low
word for error codes, full long for size; mediated CCR tests D0's low word.
The published trap words and register locations are recorded individually in
the interface registry. No header/list/zone layout or placement algorithm used.

Published numeric errors used: noErr0, memFullErr-108, nilHandleErr-109,
memWZErr-111. Published low-memory locations: HeapEnd114, TheZone118,
ApplLimit130, SysZone2A6, ApplZone2AA, Lo3Bytes31A (hex). Guest stores to
TheZone/ApplLimit affect later calls. Later manuals do not establish the original
1984 availability or behavior; earlier disposal A0 and overlap disagreements
remain research questions. No reference OS/executable was read or run.

## Independent storage and allocation decisions

Caller owns a SPEC-0022 map, one manager and reports/requests, all truthful,
live, synchronized and disjoint. Two configured even nonoverlapping writable
zones lie above400; bounds fit24 bits. Distinct nonzero even logical identities
are opaque zone tokens outside both payload spans, not copied Zone records.
Each zone reserves an original fixed128-byte band for32 master cells, indexed
by a native side-table slot. At most32 live objects across both zones; each
original record has kind/zone/address/logical-size/extent. Every metadata byte
must be charged to a future Macintosh RAM budget. No host allocation/global
mutable state. Zone expansion, InitZone layout and MoreMasters are deferred.

Initialization validates/preflights every zone/global first, then clears side
records and both cell bands and writes the six globals. No payload fill promise.
Each call validates config/map, pairwise live extents and handle cells. A live
cell's low address must match the tracked payload or be NIL; flag-byte writes
are allowed and read at call boundaries. Forged non-NIL addresses are state
failures. An externally NIL cell yields nilHandleErr for size/resize/lock calls;
its retained owned allocation is reclaimed on disposal. Purging/EmptyHandle
are deferred. Unallocated/freed objects are rejected until an address is reused;
raw historical addresses cannot distinguish a stale alias after reuse.

TheZone selects either configured identity; SYS overrides only allocation.
SysZone/ApplZone/HeapEnd/Lo3Bytes must retain configured values. ApplLimit must
be even within the application zone's payload area and configured end. Lowering
it never cuts existing objects; new/growing application extents obey it. The
fixed HeapEnd stays at the configured end; automatic growth is not implemented.
No hidden MemError global or linked Pascal interface is claimed.

Payload extents round up to even bytes, with a two-byte physical reservation
for logical size0. Negative/unrepresentable allocation/resize sizes fail with
memFullErr; capacity/slot limits also yield memFullErr. Without CLEAR and for
newly grown bytes contents are unspecified. Pointer/locked-handle growth needs
an adjacent free extent; unlocked handles may relocate to a disjoint free extent.
Shrink/preserved prefix is min(old,new); failed requests leave all manager/RAM
state unchanged. NIL/invalid/foreign/odd objects have defined engineering errors
(nilHandleErr for NIL handle queries/resize/locks; disposal and other invalid
objects use memWZErr). These are safety/profile choices, not undocumented facts.

Original bounded placement scans gaps from the zone end. If needed, an original
plan packs unlocked payloads downward in address order between fixed pointers/
locked payloads, with cells in the separate reserved band. A resized subject is
fixed while other moves are planned. This bounded strategy may fail despite
total free space if fixed barriers/retained source prevent a suitable span.
Compute placements and validate the complete request before any copy/store;
commit downward moves in ascending source order. There is no copied Apple free
list, rover, block header, purging or grow-zone algorithm. Failed plans do not
move other blocks. Only allocation/resize may compact in this profile.

Lock/unlock is idempotent, changes only bit31 of the cell and preserves other
flag bits. GetSize and BlockMove never relocate. BlockMove preflights the whole
source/destination byte span, copies overlap in the correct direction, and
defines count0 as a successful no-op without address dereference. Negative
count is an engineering memWZErr. No MMIO, callbacks or interrupt inference.

## API and register boundary

`cs_mm_execute` distinguishes argument/state/map/operation/flags/interrupt
contract failures from successful API dispatch with a Macintosh-style error.
Contract failure preserves reply, manager and RAM; API errors write an explicit
reply only. Interrupt context is explicit caller input and refuses all calls.
Reports may change. No native undefined access from a guest argument.

The separate `cs_mm_register_call` supports eleven reviewed OS routine numbers,
SYS/CLEAR only on allocation, and bit8 A0 preservation. It calls the same core,
updates only D0/A0/CCR, leaves PC/A7 and all other registers intact. Word errors
preserve D0's high word, size success writes a full long, and size-query errors
are explicitly sign-extended as an engineering choice. CCR preserves X/upper
SR, updates N/Z from D0.low16 and clears V/C. Unsupported formats/flags and
contract failures leave the value frame untouched. This boundary is not yet
installed in SPEC-0020's address directory or a CPU exception/boot entry.
Get/SetTrapAddress remain unchanged and reject their prior unsupported flags.
No x86 application SDK/headers or native application is produced.
All address parameters here are strict logical addresses. Normalizing tagged
68000 data addresses is a separate explicit boundary contract before CPU/trap
integration; this C/register adapter does not silently mask its arguments.

## Verification and remaining gates

Independent allocation interval/guard checks, logical-versus-physical sizes,
slot/full/negative/zero cases, guest zone/limit changes, compaction, prefix and
failed-resize atomicity, direct flag writes, stale/forged cells, copy overlap,
register/word-test behavior and two independent managers. Run strict Clang/GCC
width/optimization, sanitizer and endian object checks; GNU68000 standalone
linkage/import/stack reports are compiler evidence, not execution or final ROM
budget. Registry validation checks each marshalling contract independently.
No boot image changes or VM/emulator launch. Native application gate, source
review, January1984 confirmation, physical/full budgets/parity/three editions
and B2 criterion6 stay open. Later steps bind the typed gateway, normalize the
68000 data-bus address policy and execute original guest heap probes.
