# Memory Manager first-subset draft — 2026-10-12

Draft research contract prepared by Claude at the implementation lead's request.
**Not a specification**: no SPEC/ADR/TEST number, no acceptance claim, no code.
The implementation lead reviews it and allocates identifiers. Companion:
[test plan](memory-manager-test-plan.md).

## Sources and labels

- **SRC-0071**, *Inside Macintosh* Volumes I–III (Addison-Wesley, 1985), the
  owner's copy. Cited as printed page, then PDF page of that copy (Volume I
  PDF = page + 10; Volume II PDF = page + 568; Volume III PDF = page + 1003).
  It describes ROM version 105 and the May 1984 System file (I-4).
- **SRC-0068**, *Inside Macintosh* Promotional Edition (March 1985), catalog
  copy SHA-1 `4ed5b3c2…f13c5`. Its Memory Manager chapter is dated 10/9/84.
  Cited as `PE MM-n` (chapter page), then PDF page (= n + 714).
- Both are **later manuals**. Nothing here confirms January 1984 behavior.
- No sample code, equate file, Apple heap algorithm or internal heap layout is
  reproduced. Block-header, zone-record and master-pointer-block internals
  (II-20–25) were read only to know what exists; they are not adopted.

Labels used throughout:

- **[DOC85]** documented in SRC-0071; **[DOCPE]** documented in SRC-0068.
- **[ENG]** a ClasSICK engineering proposal, not historical behavior.
- **[OPEN]** not documented, or the editions disagree; needs a decision or a
  reference observation.
- **[DEC-n]** a decision needed from the implementation lead (listed at the end).

## Proposed minimal first subset

Phase 1 (this draft's contracts): **NewPtr, DisposPtr, GetPtrSize,
SetPtrSize, NewHandle, DisposHandle, GetHandleSize, SetHandleSize, HLock,
HUnlock, BlockMove**, the SYS and CLEAR trap-word flags, and result reporting.

Why these: they are the allocation, lifetime, size, locking and copy primitives
every later manager needs; all eleven are register-based Operating System traps
with documented trap words in both editions' summaries; none needs a guest
callback; and their observable effects can be tested in hosted C before any
broader CPU work.

Deferred, with reasons, in "Implementation sequence" below: purging
(HPurge, HNoPurge, EmptyHandle, ReallocHandle), RecoverHandle, MoreMasters,
zone access and creation, free-space queries, compaction and reservation
controls, grow-zone functions, and application-zone setup.

**Availability and naming [DOC85][DOCPE]:** the routine names above are the
documented spellings (`DisposPtr`, `DisposHandle`, not "Dispose…"). Each has a
trap macro in the 1985 summary (II-47–50, PDF 615–618) and in Appendix C
(III-215–220), and in the 10/9/84 chapter (PE MM-32–47). **[OPEN]** Neither
source shows that each trap exists, with the same number, in the January 1984
ROM; that needs reference observation.

**Not traps [DOC85][DOCPE]:** MemError, MaxApplZone, GetApplLimit, SystemZone,
ApplicZone, GZSaveHnd, TopMem and MoveHHi are marked "[Not in ROM]" (II-29–44,
PDF 597–612); the 10/9/84 chapter marks MemError and TopMem "[No trap macro]"
(PE MM-47–48, PDF 761–762). They are interface code linked into applications.
ClasSICK supports them for guest code by keeping the documented low-memory
globals they read correct (see "Guest-visible state"), not by adding traps.

## API evidence matrix

All phase-1 routines are Operating System format (bit 11 = 0). Register
conventions on return from the trap follow SPEC-0017's register-based rules:
A1, D1 and D2 restored; A0 restored when trap bit 8 is 0, otherwise the result;
condition codes from a word test of D0's low 16 bits (I-94–95, PDF 104–105;
II-27, PDF 595).

| Routine | Trap word (Appendix C) | On entry → on exit | Result codes | Moves or purges memory (Appendix B) | 10/9/84 cross-check |
| --- | --- | --- | --- | --- | --- |
| NewPtr | A11E (III-218, PDF 1221); bit 8 set | D0 size (long) → A0 pointer or NIL, D0 code | noErr, memFullErr | Yes (III-212) | PE MM-36 (PDF 750): same |
| DisposPtr | A01F (III-216, PDF 1219) | A0 pointer → D0 code | noErr, memWZErr | Yes (III-211) | PE MM-37 (PDF 751) also lists "A0: 0" on exit **[OPEN, DEC-9]** |
| GetPtrSize | A021 (III-217, PDF 1220) | A0 pointer → D0 size if ≥ 0, else result code | noErr (Pascal only), memWZErr; Pascal result 0 on error | No | PE MM-37: same |
| SetPtrSize | A020 (III-220, PDF 1223) | A0 pointer, D0 size → D0 code | noErr, memFullErr, memWZErr | Yes (III-213) | PE MM-38 (PDF 752): same codes |
| NewHandle | A122 (III-218); bit 8 set | D0 size → A0 handle or NIL, D0 code | noErr, memFullErr | Yes (III-212) | PE MM-32 (PDF 746): same |
| DisposHandle | A023 (III-216) | A0 handle → D0 code | noErr, memWZErr | Yes (III-211) | PE MM-33 (PDF 747) also lists "A0: 0" **[OPEN, DEC-9]** |
| GetHandleSize | A025 (III-217) | A0 handle → D0 size if ≥ 0, else result code | noErr (Pascal only), nilHandleErr, memWZErr; Pascal result 0 on error | No | PE MM-33–34: same |
| SetHandleSize | A024 (III-219, PDF 1222) | A0 handle, D0 size → D0 code | noErr, memFullErr, nilHandleErr, memWZErr | Yes (III-213) | PE MM-34 (PDF 748): same |
| HLock | A029 (III-217) | A0 handle → D0 code | noErr, nilHandleErr, memWZErr | No | PE MM-42 (PDF 756): same, plus a note that setting the master pointer's lock bit directly is faster |
| HUnlock | A02A (III-217) | A0 handle → D0 code | noErr, nilHandleErr, memWZErr | No | PE MM-43 (PDF 757): same |
| BlockMove | A02E (III-215, PDF 1218) | A0 source, A1 destination, D0 count → D0 code | noErr | No | PE MM-47 (PDF 761): same, **without** the 1985 statement that overlap works **[OPEN, DEC-10]** |

Routine pages in SRC-0071: NewHandle II-32 (PDF 600); DisposHandle and
GetHandleSize II-33 (PDF 601); SetHandleSize II-34 (PDF 602); NewPtr and
DisposPtr II-36 (PDF 604); GetPtrSize and SetPtrSize II-37 (PDF 605); HLock and
HUnlock II-41 (PDF 609); BlockMove II-44 (PDF 612); register summary II-49–50
(PDF 617–618).

Result code values **[DOC85]** (II-46, II-48, PDF 614, 616): noErr 0,
memFullErr −108, nilHandleErr −109, memWZErr −111, memPurErr −112,
memLockedErr −117. The 10/9/84 summary (PE MM-53, PDF 767) lists the same
values except memLockedErr, and adds a constant `maxSize = $800000` (maximum
block size) that the 1985 summary omits **[OPEN]**.

Trap-word flags **[DOC85][DOCPE]** (II-27, PDF 595; PE MM-26, PDF 740): bit 10
(SYS) makes "some" routines use the system heap zone instead of the current
zone; bit 9 (CLEAR) makes NewPtr and NewHandle zero the new block. Each routine
description lists which apply; in phase 1, SYS and CLEAR apply to NewPtr and
NewHandle only.

## Behavioral contracts

### C1 Representation visible to callers

1. **[DOC85]** A new block is referred to by the address of its contents, not
   its header (II-11, PDF 579). A pointer is that address for a
   nonrelocatable block. A handle is the address of a **master pointer**: a
   nonrelocatable 4-byte cell holding the relocatable block's contents address
   (II-12, PDF 580). NIL is 0.
2. **[DOC85]** A handle whose master pointer is NIL is **empty** (purged);
   handles to it stay valid (II-14, PDF 582; II-40, PDF 608).
3. **[DOC85][DOCPE]** Blocks are aligned on even addresses, so guest code may
   use word and long accesses (II-10, PDF 578; PE MM-5, PDF 719).
4. **[DOC85]** The master pointer's high-order byte carries lock, purge and
   resource flags, and its low three bytes the address; the book warns the flags
   affect arithmetic, and the global `Lo3Bytes` holds $00FFFFFF (II-25, PDF 593;
   III-228). The 10/9/84 HLock and HUnlock notes tell programmers they may set
   or clear the lock bit directly (PE MM-42–43). **[DEC-1]** whether ClasSICK
   adopts this guest-visible representation; it interacts with DEC-2.
5. **[ENG]** Size is a signed 32-bit long (`Size = LONGINT`, II-18, PDF 586).

### C2 Allocation (NewPtr, NewHandle)

1. **[DOC85]** On success: a non-NIL pointer or handle in A0, D0 = noErr. A new
   relocatable block is unlocked and unpurgeable (II-11, PDF 579; II-32).
2. **[DOC85]** With CLEAR, every byte of the new block is zero (II-27).
   Without CLEAR, initial contents are not specified **[OPEN, DEC-6]**.
3. **[DOC85]** With SYS, the block comes from the system heap zone, not the
   current zone (II-27).
4. **[DOC85]** The new block lies in a heap zone, and every byte of a zone
   belongs to exactly one block (II-10, PDF 578). Therefore live blocks and
   master pointers never overlap one another. ClasSICK tests this as a
   non-overlap property.
5. **[DOC85]** On failure: NIL in A0, D0 = memFullErr (II-32, II-36). Before
   failing, the original compacts, grows the application zone, purges and calls
   the grow-zone function (II-12–14, PDF 580–582). **[ENG]** Phase 1 may move
   unlocked relocatable blocks to make room, using an original strategy; it does
   not purge or call grow-zone functions until those phases exist. Apple's
   placement order ("first fit", rover position, lowest-address placement of
   nonrelocatable blocks) is not a ClasSICK contract.
6. **[OPEN, DEC-5]** Zero, negative and very large sizes are not documented in
   either edition (the 10/9/84 `maxSize` constant only names a maximum).

### C3 Lifetime (DisposPtr, DisposHandle)

1. **[DOC85]** Disposal releases the block's memory; afterwards every pointer
   or handle to it is invalid, and disposing an invalid one again "will damage
   the master pointer list" (II-33, II-36). Error result: memWZErr.
2. **[OPEN, DEC-4]** Using an invalid, foreign or NIL pointer or handle is not
   specified historically. **[ENG]** ClasSICK must never corrupt native state
   or crash the host because of one; detection and the result returned are an
   engineering decision, labeled as such.
3. **[OPEN, DEC-9]** A0 after DisposPtr/DisposHandle: the 10/9/84 chapter says
   0; the 1985 trap words (A01F, A023, bit 8 clear) and the register rule imply
   A0 is restored to the argument.

### C4 Size queries (GetPtrSize, GetHandleSize)

1. **[DOC85]** Return the logical size (the size requested or last set), not
   the space consumed (II-33, II-37; II-25 explains blocks may hold unused
   bytes).
2. **[DOC85]** From assembly, D0 holds the size when ≥ 0, else a result code;
   the dispatcher's word test therefore sets condition codes incorrectly for
   long sizes (for example, 65536 gives Z = 1). Pascal callers get 0 on error.
3. **[OPEN]** Whether a negative result code in D0 is sign-extended to 32 bits.

### C5 Resizing (SetPtrSize, SetHandleSize)

1. **[DOC85]** Changes the logical size; GetPtrSize/GetHandleSize then report
   the new size (II-34, II-37).
2. **[DOC85]** Growing a locked relocatable block may fail, because a
   nonrelocatable or locked block may lie above it (II-34). Failure is
   memFullErr. The 10/9/84 chapter says the same and also warns against
   growing locked blocks in SetPtrSize's note.
3. **[OPEN]** Neither edition states that contents up to the smaller size are
   preserved, or that a failed resize leaves the block unchanged (only
   ReallocHandle states the latter, II-35). **[ENG, DEC-7]** ClasSICK adopts
   both: preserve the first min(old, new) bytes; on any failure leave address,
   size and contents unchanged.
4. **[DOC85]** SetHandleSize on an empty handle returns nilHandleErr (II-34).

### C6 Movement and locking

1. **[DOC85]** Nonrelocatable blocks never move (II-10, II-12).
2. **[DOC85]** Relocatable blocks move only during the routines listed in
   Appendix B (II-11, PDF 579; III-211–213, PDF 1214–1216). Of phase 1, NewPtr,
   NewHandle, DisposPtr, DisposHandle, SetPtrSize and SetHandleSize are listed;
   GetPtrSize, GetHandleSize, HLock, HUnlock and BlockMove are not.
3. **[DOC85]** When a block moves, its master pointer is updated; the handle
   (the master pointer's address) does not change, and the block is copied whole
   (II-12, II-14).
4. **[DOC85]** HLock prevents a relocatable block from moving; HUnlock allows it
   again; both do nothing if already in that state (II-41, PDF 609).
5. **[DOC85]** Memory Manager routines must not be called from interrupt code
   (II-27; III-211). **[ENG]** ClasSICK detects such a call when it can and
   fails it deterministically.
6. **[OPEN]** Appendix B omits MaxMem, although MaxMem compacts and purges
   (II-38). Treat MaxMem as memory-moving when it is implemented.

### C7 BlockMove

1. **[DOC85]** Copies byteCount bytes from source to destination; updates no
   pointers; correct even when the ranges overlap (II-44, PDF 612).
2. **[OPEN, DEC-10]** The 10/9/84 text does not mention overlap.
3. **[OPEN]** A zero or negative count is not documented. **[ENG]** Zero is a
   no-op returning noErr.

### C8 Result reporting

1. **[DOC85]** Routines that return a code leave it in D0's low word; the
   dispatcher's word test sets the condition codes (II-27).
2. **[DOC85]** Pascal interface code stores the result for MemError and
   "manufactures" noErr when a trap returns none (II-27); MemError itself is not
   a trap (II-44). **[ENG]** The native C API keeps an explicit per-call result
   rather than a hidden global.

## Guest-visible state

**[DOC85]** Globals the Memory Manager owns or uses (Appendix D, III-227–231;
II-51, PDF 619; addresses as in `system1-low-memory.md`):

| Global | Address | Meaning |
| --- | --- | --- |
| HeapEnd | $114 | End of application heap zone |
| TheZone | $118 | Current heap zone |
| ApplLimit | $130 | Application heap limit |
| SysZone | $2A6 | System heap zone |
| ApplZone | $2AA | Application heap zone |
| Lo3Bytes | $31A | Constant $00FFFFFF |
| MinStack, DefltStack | $31E, $322 | Stack allotments |
| GZRootHnd | $328 | Block the grow-zone function must not move |

**[DOC85]** Guest code may change the current zone by storing into TheZone,
and the heap limit by storing into ApplLimit (II-30–31, PDF 598–599). **[ENG]**
So ClasSICK must read such globals from guest memory on each call, treating the
guest copy as the authority, rather than caching them natively.

## CPU and guest-memory boundary

What the Memory Manager needs from the CPU and guest-memory work, without
choosing its design:

1. **Checked data access.** Byte, word and long reads and writes of guest RAM,
   big-endian, with bounds checks and no native pointer exposure (AGENTS rule).
   SPEC-0021 today exposes only an executable span and a stack span; the heap,
   master pointers and low-memory globals need a general data span.
2. **Addresses with a high byte.** The 68000 addresses 16 MB (III-17), and
   documented master pointers carry flags in the high byte (II-25). Guest code
   that dereferences a master pointer directly would present a 32-bit value
   with a nonzero high byte. SPEC-0021 refuses high address bits.
   **[DEC-2]** mask to 24 bits for data access, or keep the refusal and adopt
   out-of-band flags under DEC-1.
3. **Alignment faults.** Word and long accesses at odd addresses fault on the
   68000 (Motorola, SRC-0093 locator to record). Returned addresses are even
   (C1.3), so faults indicate guest errors.
4. **Permissions.** ROM read-only; RAM read-write including low-memory globals
   (guest code writes TheZone and ApplLimit).
5. **Persistent state.** Heap contents, master pointers and globals live in
   guest RAM and survive across traps. The system zone survives application
   relaunch; the application zone is reinitialized for each application (II-10,
   PDF 578). **[DEC-3]** where ClasSICK keeps its own block metadata: inside
   guest RAM with an original layout, or in a native side table. Either choice
   changes how much memory guests see (see budget).
6. **Error propagation.** A guest-memory fault raised while a trap is running
   (for example, a handle pointing outside RAM) must come back as a defined
   ClasSICK result, never native undefined behavior (DEC-4).
7. **Trap gateway.** SPEC-0020 rejects OS bits 9–10 for its intrinsics. Memory
   Manager traps need them routed per routine (SYS, CLEAR) **[DEC-8]**, plus A0
   and D0 marshalling, A1/D1/D2 restoration and the D0 word test.
8. **Callbacks.** Grow-zone functions are guest routines called during
   allocation (II-42–43). They wait for guest callback execution (SPEC-0017).

## Macintosh 128K budget notes

- **[DOC85]** System heap typically 16.75 KB on a 128K (II-9, PDF 577);
  **[DOCPE]** 16.5 KB (PE MM-4, PDF 718). The value comes from the startup
  disk's system startup information (II-9). **[OPEN]** January 1984 value;
  parameterize it.
- **[DOC85][DOCPE]** The application zone starts at 6 KB and grows on demand up
  to the application heap limit (II-10, II-13), in 1 KB steps (II-13), leaving
  8 KB for the stack by default (II-17) and never within 1 KB of the stack's
  end (II-18).
- **[ENG]** ClasSICK's per-block overhead will differ from Apple's undocumented
  internal choices. Under memory pressure that changes how many blocks fit and
  what FreeMem would report. Track the overhead as a budget item; do not copy
  Apple's header layout to match it.
- **[ENG]** On the 128K, any native side table (DEC-3) consumes the same RAM as
  the guest view; it must fit inside the system globals and system heap budget
  from the 128K feasibility study.

## Implementation sequence

1. **Hosted C core (no CPU).** An original Memory Manager over a caller-supplied
   guest-RAM byte buffer with logical addresses: zones, nonrelocatable and
   relocatable blocks, master pointers, locking, moving unlocked blocks, result
   codes. Most of the test plan runs here.
2. **Representation decisions.** DEC-1 to DEC-3, then the guest-visible globals
   (TheZone, SysZone, ApplZone, HeapEnd, ApplLimit, Lo3Bytes) in guest memory.
3. **Trap binding.** Register-based adapters for the eleven phase-1 traps
   through the SPEC-0020/0021 boundary, with flags, A0/D0, register restoration
   and the word test (DEC-8).
4. **CPU growth.** Data-memory addressing modes in the interpreter, so guest
   probes can dereference handles and touch blocks (DEC-2).
5. **Purging and recovery.** HPurge, HNoPurge, EmptyHandle, ReallocHandle,
   RecoverHandle, MoreMasters, HandleZone, PtrZone, GetZone, SetZone.
6. **Zones and launch.** InitApplZone, SetApplBase, InitZone, SetApplLimit,
   application-zone growth and stack limits, tied to the Segment Loader.
7. **Space control.** FreeMem, MaxMem, CompactMem, PurgeMem, ResrvMem,
   SetGrowZone with guest callbacks.
8. **Observation.** Native Mac and PC runs, then authorized January 1984
   reference observations for every OPEN item.

What can wait: grow-zone callbacks, heap zones on the stack (II-45), purge
warning procedures, the resource flag (Resource Manager), MoveHHi (not a ROM
trap), and 512K values.

## Decisions needed from the implementation lead

| ID | Decision |
| --- | --- |
| DEC-1 | Adopt the documented master-pointer flag byte (II-25) as guest-visible, so programs that set the lock bit directly behave, or keep flags out-of-band |
| DEC-2 | CPU data access: mask addresses to 24 bits, or keep refusing high bits |
| DEC-3 | Block metadata inside guest RAM (original layout) or in a native side table |
| DEC-4 | Policy for invalid, freed, foreign or NIL pointers and handles |
| DEC-5 | Zero, negative and oversized requests |
| DEC-6 | Initial contents of blocks allocated without CLEAR (deterministic fill or unspecified) |
| DEC-7 | Adopt content preservation and unchanged-on-failure for SetPtrSize/SetHandleSize |
| DEC-8 | Accept OS trap bits 9–10 per routine for Memory Manager traps |
| DEC-9 | A0 after DisposPtr/DisposHandle: 1985 register rule (restored) or 10/9/84 text (0) |
| DEC-10 | BlockMove overlap: adopt the 1985 overlap-safe rule pending 1984 observation |
| DEC-11 | System heap size for `system1-1984`: parameterize between 16.5 KB and 16.75 KB until observed |
