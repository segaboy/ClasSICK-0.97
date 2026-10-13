# Memory Manager first-subset test plan — 2026-10-12

Draft test plan prepared by Claude to accompany the
[Memory Manager draft](memory-manager-draft.md). Not a TEST record; no
identifiers or acceptance claims. Every case is independently authored from
the documented behavior; no Apple sample code, heap layout or fixture is used.

## Conventions

- **Layer** says where a case can run first: **H** hosted C contract over a
  caller-supplied guest-RAM buffer; **G** guest trap path (needs the trap
  binding and, for dereferencing, CPU data access); **N** native PC/Mac image;
  **R** authorized January 1984 reference observation.
- **Source** cites SRC-0071 pages (with PDF page of the owner's copy) or the
  10/9/84 chapter of SRC-0068 (`PE MM-n`). Labels as in the draft: [DOC85],
  [DOCPE], [ENG] for ClasSICK choices, **OPEN** where the expected result is not
  documented. OPEN cases record what to observe, not an invented answer.
- **Fixture zone:** unless stated, one application zone of 16 KiB inside a
  64 KiB guest-RAM buffer, base at a nonzero even logical address, guard bytes
  of $DB filling the rest of the buffer, current zone = that zone, no other
  allocations. A second 4 KiB zone stands in for the system zone where SYS is
  tested.
- **Sentinels:** in G cases, D1 = $11111111, D2 = $22222222, A1 = $A1A1A1A1,
  D3–D7 and A2–A6 distinct values, checked after every trap.
- **Every case** also checks that guard bytes outside the zones are unchanged
  and that no native error was raised.

## Allocation

**MM-T01 NewPtr succeeds.** H, G.
Initial: fixture zone. Operation: NewPtr(16).
Expected: A0 ≠ 0, A0 even, A0..A0+15 inside the zone; D0 = 0 (noErr); in G,
condition codes Z = 1, N = 0; GetPtrSize(A0) = 16.
Failure behavior: none expected.
Source: II-36 (PDF 604), II-10 (PDF 578), II-37 (PDF 605); PE MM-36–37.

**MM-T02 NewHandle succeeds.** H, G.
Initial: fixture zone. Operation: NewHandle(16).
Expected: handle h ≠ 0 and even; the long at h (master pointer) ≠ 0 and even;
GetHandleSize(h) = 16; D0 = 0. The new block is unlocked: HUnlock(h) returns
noErr and changes nothing; a following move-capable call may move it (MM-T19).
Source: II-32 (PDF 600), II-11–12 (PDF 579–580); PE MM-32.

**MM-T03 CLEAR zeroes the block.** H, G.
Initial: fill the zone's free space with $A5 through the harness.
Operation: NewPtr with CLEAR (bit 9), size 64; NewHandle with CLEAR, size 64.
Expected: all 64 bytes of each block are $00.
Without CLEAR: no expectation about contents (OPEN, DEC-6).
Source: II-27 (PDF 595); PE MM-26 (PDF 740).

**MM-T04 SYS selects the system zone.** H, G.
Initial: both fixture zones; current zone = application zone.
Operation: NewPtr with SYS, size 32; then NewHandle with SYS, size 32.
Expected: the pointer and the handle's block lie inside the system zone;
TheZone still names the application zone afterward.
Source: II-27; II-10 (PDF 578).

**MM-T05 Odd sizes keep addresses even.** H.
Operation: NewPtr(7), NewPtr(1), NewHandle(3), NewHandle(5).
Expected: GetPtrSize/GetHandleSize report 7, 1, 3, 5; every returned address
and master pointer value is even.
Source: II-10; II-25 (PDF 593) even physical sizes; PE MM-5.

**MM-T06 No overlap.** H.
Operation: allocate a deterministic mix (pointer and handle, sizes 2–600 from
a fixed sequence) until a request fails with memFullErr.
Expected: all live blocks' [address, address + logical size) ranges are
pairwise disjoint, disjoint from every master-pointer cell, and inside the zone.
Source: II-10 (every byte belongs to exactly one block).

**MM-T07 Allocation failure.** H, G.
Initial: fixture zone with three locked handles and two pointers holding known
patterns. Operation: NewPtr(1,000,000); NewHandle(1,000,000).
Expected: A0 = 0; D0 = −108 (memFullErr); in G, N = 1, Z = 0. Existing pointers
keep their addresses and contents; locked handles keep their master pointer
values and contents; unlocked handles may move but keep contents.
Source: II-32, II-36; II-12–14 (PDF 580–582).

**MM-T08 Size extremes.** H. **OPEN.**
Operation: NewPtr(0), NewHandle(0), NewPtr(−1), NewPtr($800000).
Expected: not documented (DEC-5). Record ClasSICK's chosen result as [ENG];
schedule R observation. The 10/9/84 `maxSize = $800000` constant is context
only (PE MM-53, PDF 767).

## Lifetime

**MM-T09 Disposal reclaims space.** H.
Operation: allocate pointers of 100 bytes until memFullErr (count n); dispose
all; repeat the same allocations.
Expected: the second round also allocates n pointers. Each DisposPtr returns 0.
Source: II-36 ("releases the memory"); [ENG] for the exact-count equality,
since placement is not specified.

**MM-T10 DisposHandle succeeds.** H, G.
Operation: h := NewHandle(40); DisposHandle(h).
Expected: D0 = 0. In G, A0 after the trap: OPEN (DEC-9); record it.
Source: II-33 (PDF 601); PE MM-33 lists "A0: 0".

**MM-T11 Invalid or stale arguments.** H, G. **OPEN.**
Operations: DisposPtr(p) twice; DisposHandle(h) twice; GetHandleSize on a
disposed handle; DisposPtr(0); DisposHandle(0); an odd pointer; a pointer into
the middle of a block; an address outside RAM.
Expected historically: undefined ("will damage the master pointer list",
II-33, II-36). [ENG] Required regardless: no native fault, no write outside
the zone, guard bytes intact, a defined ClasSICK result per DEC-4.

## Size queries

**MM-T12 Sizes track changes.** H, G.
Operation: h := NewHandle(10); SetHandleSize(h, 300); GetHandleSize(h);
SetHandleSize(h, 4); GetHandleSize(h).
Expected: 300, then 4; each D0 = 0 for the SetHandleSize calls.
Source: II-33–34 (PDF 601–602).

**MM-T13 Long size and the word test.** G.
Initial: zone large enough for 65,536 bytes (use a 128 KiB fixture).
Operation: p := NewPtr(65536); GetPtrSize(p).
Expected: D0 = $00010000; condition codes Z = 1 because the dispatcher tests
only D0's low word. Repeat with GetHandleSize.
Source: II-33, II-37 (PDF 605) assembly notes; PE MM-33, MM-37.

**MM-T14 Error result encoding.** G. **OPEN.**
Operation: GetHandleSize on an empty handle (after phase 5) or a freed handle
(DEC-4).
Expected: D0 negative result code (nilHandleErr −109 for an empty handle);
whether the high word is sign-extended is OPEN; Pascal-visible result 0.
Source: II-33.

## Resizing

**MM-T15 Grow with move, contents kept.** H, G.
Initial: h := NewHandle(16) filled with $00..$0F; then p := NewPtr(32) placed
so the block above h is nonrelocatable (harness arranges by allocation order;
if the allocator places p elsewhere, add pointers until one is adjacent).
Operation: SetHandleSize(h, 1000).
Expected: D0 = 0; GetHandleSize(h) = 1000; the handle value h is unchanged; the
master pointer may change; the first 16 bytes still read $00..$0F.
Source: II-34; II-12, II-14; content preservation is [ENG] (DEC-7).

**MM-T16 Shrink keeps the prefix.** H, G.
Operation: h := NewHandle(100) filled with a counter; SetHandleSize(h, 10);
SetPtrSize on a 100-byte pointer to 10.
Expected: sizes 10; first 10 bytes unchanged; pointer address unchanged.
Source: II-34, II-37; preservation [ENG] (DEC-7).

**MM-T17 Growing a locked block may fail cleanly.** H, G.
Initial: h := NewHandle(32), HLock(h); a nonrelocatable block directly above.
Operation: SetHandleSize(h, 4000).
Expected: either D0 = 0 with contents kept, or D0 = −108. On failure, size,
master pointer and contents are unchanged ([ENG] DEC-7).
Source: II-34 note; PE MM-34 note.

**MM-T18 SetPtrSize growth blocked.** H, G.
Initial: p := NewPtr(32), then another pointer directly above.
Operation: SetPtrSize(p, 4000).
Expected: D0 = 0 or −108; p never changes (nonrelocatable); on failure size and
contents unchanged ([ENG]).
Source: II-37; II-10, II-12.

## Movement and locking

**MM-T19 Unlocked blocks may move; handles do not.** H.
Initial: 20 unlocked handles of 50 bytes with distinct patterns; dispose every
other one; then request a block larger than any single gap.
Expected: allocation succeeds if total free space allows; each surviving handle
value is unchanged; master pointers may change; every surviving block's
contents are intact.
Source: II-12, II-14; [ENG] for success when total free space suffices
(compaction strategy is original).

**MM-T20 Locked and nonrelocatable blocks never move.** H.
Initial: as MM-T19, plus HLock on five handles and three pointers.
Operation: 200 deterministic move-capable calls (NewHandle, SetHandleSize,
DisposHandle, NewPtr, DisposPtr).
Expected: the locked handles' master pointer values and all pointer values are
unchanged throughout; their contents unchanged.
Source: II-10, II-12, II-41 (PDF 609).

**MM-T21 Calls outside Appendix B never move anything.** H, G.
Operation: GetHandleSize, GetPtrSize, HLock, HUnlock on every block; BlockMove
between two blocks.
Expected: no master pointer value changes; no block contents change except the
BlockMove destination.
Source: II-11 (PDF 579); III-211–213 (PDF 1214–1216).

**MM-T22 Lock and unlock are idempotent.** H, G.
Operation: HLock(h) twice; HUnlock(h) twice.
Expected: every D0 = 0; after the second HUnlock the block may move again.
Source: II-41.

**MM-T23 Lock state in the master pointer.** G. **Depends on DEC-1/DEC-2.**
Operation: guest code sets bit 7 of the master pointer's high byte (as the
10/9/84 note allows), then runs move-capable calls; separately, HLock(h) and
read the high byte.
Expected if DEC-1 adopts the documented representation: the block does not
move after the guest sets the bit, and HLock sets bit 7; dereferencing the
flagged master pointer still reaches the block (needs DEC-2 masking).
Source: II-25 (PDF 593); PE MM-42–43; III-17.

**MM-T24 Interrupt-context call.** H, N. [ENG]
Operation: call NewPtr while the harness marks interrupt context.
Expected: a deterministic ClasSICK failure; zone unchanged.
Source: II-27; III-211 (the restriction itself; the failure mode is [ENG]).

## BlockMove

**MM-T25 Plain copy.** H, G.
Operation: BlockMove(src, dst, 37) between non-overlapping blocks.
Expected: dst holds src's 37 bytes; src unchanged; D0 = 0; no pointer or master
pointer changes.
Source: II-44 (PDF 612); PE MM-47.

**MM-T26 Overlap both directions.** H, G.
Operation: in one 100-byte block holding $00..$63, BlockMove(b, b+10, 50), then
reset and BlockMove(b+10, b, 50).
Expected (1985): results equal a copy through a temporary buffer.
1984: OPEN (the 10/9/84 text is silent; DEC-10). Schedule R observation.
Source: II-44.

**MM-T27 Zero count.** H. **OPEN** historically; [ENG] no bytes change, D0 = 0.

## Register and result conventions

**MM-T28 Register preservation for each phase-1 trap.** G.
Operation: call each of the eleven traps with sentinels loaded.
Expected: D1, D2, A1 equal their sentinels afterward; D3–D7 and A2–A6
unchanged; A0 is the result for NewPtr and NewHandle (trap bit 8 set) and the
argument for the others (bit 8 clear), except DisposPtr/DisposHandle per DEC-9;
condition codes N and Z match D0's low word.
Source: I-94–95 (PDF 104–105); II-27; Appendix C trap words (III-215–220).

**MM-T29 Flags routed only where documented.** G. [ENG, DEC-8]
Operation: issue DisposPtr and HLock with bit 9 or bit 10 set.
Expected: OPEN historically; ClasSICK result per DEC-8, documented in the
binding.

## Guest-visible globals

**MM-T30 TheZone written by guest code.** G (phase 5).
Operation: store the system zone's address into TheZone ($118), then NewPtr(8).
Expected: the block comes from the system zone.
Source: II-31 (PDF 599); III-231.

**MM-T31 Globals stay current.** H, G.
Operation: after initialization and after each test above, read HeapEnd, TheZone,
SysZone, ApplZone, ApplLimit and Lo3Bytes from guest memory.
Expected: Lo3Bytes = $00FFFFFF; TheZone names the current zone; SysZone and
ApplZone name their zones; HeapEnd does not exceed ApplLimit unless the limit
was lowered below an already larger zone, which SetApplLimit does not cut back
(II-30, PDF 598).
Source: III-227–231; II-51 (PDF 619).

## Later phases (outline only)

- Purging (II-14, II-40–42): an unlocked purgeable block is purged under
  pressure, its master pointer becomes 0 and the handle stays valid; a locked
  purgeable block is not purged until unlocked; EmptyHandle on a locked block
  returns memPurErr (−112); ReallocHandle failure leaves the handle unchanged
  (II-35).
- RecoverHandle leaves D0 unchanged (II-35).
- MaxMem compacts and purges (II-38) although Appendix B omits it (OPEN).
- System zone survives an application relaunch; the application zone does not
  (II-10).
- Grow-zone function called repeatedly until it returns 0, then memFullErr
  (II-43).

## Reference observations to schedule (R)

For the January 1984 profile, under an authorized black-box protocol: trap
presence and numbers for the eleven routines; zero and extreme sizes; A0 after
DisposPtr/DisposHandle; BlockMove overlap; sign extension of size-query error
codes; system heap size on a 128K; whether guest-set lock bits are honored.
