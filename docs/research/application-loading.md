# Application loading research — 2026-10-12

Research record prepared by Claude at the owner's request. It is not a
specification and changes no code; the implementation lead decides what to adopt
and allocates identifiers. It traces what has to happen between a 1984 program
on a disk and its first instruction running in ClasSICK's 68000 interpreter, so
the PC port's application-runtime work (M0.6) can be planned.

## Sources and labels

- **SRC-0071**, owner's *Inside Macintosh* Volumes I–III (1985). Printed page,
  then PDF page of that copy (Volume I PDF = page + 10, Volume II = page + 568,
  Volume III = page + 1003). Describes ROM version 105 and the May 1984 System
  file.
- **SRC-0068**, *Inside Macintosh* Promotional Edition (March 1985), catalog
  copy SHA-1 `4ed5b3c2…f13c5`. Chapters cross-checked: Segment Loader (8/24/84,
  chapter page n = PDF n + 776), Resource Manager (11/28/84), and *The Structure
  of a Macintosh Application* (2/8/84), the earliest-dated chapter found on this
  topic.
- Both are later manuals. Labels: **[DOC85]**, **[DOCPE]** documented;
  **[INF]** inferred from documented facts; **[ENG]** ClasSICK proposal;
  **[OPEN]** undocumented or editions differ.
- Formats are described from the published tables. No Apple sample code, boot
  blocks, disk images, application files or resources are inputs.

## The path in one view

1. Read the program file from an MFS volume (disk format).
2. Parse its resource fork (resource file format).
3. Read `'CODE'` resource 0: application-space sizes and the jump table.
4. Reset the application heap, lay out the A5 world and stack, and copy in the
   jump table.
5. Execute the first jump-table entry. It calls `_LoadSeg`, which loads
   `'CODE'` 1 (the main segment), patches that segment's jump-table entries and
   jumps into it.
6. Later calls to routines in other segments go through their jump-table
   entries the same way.

Every step below cites its pages, and says what the CPU, trap gateway and other
managers must provide.

## 1. MFS volume format

**[DOC85]** II-119–123 (PDF 687–691):

- Volumes are read in 512-byte logical blocks; on Macintosh-initialized
  volumes each block also carries 12 bytes of Disk Driver information
  (II-119–120).
- Logical blocks 0–1 hold **system startup information** (configurable
  parameters such as event-queue capacity, initial system heap size and open-file
  count), zero if not a startup disk (II-120). **[OPEN]** Their byte layout is
  not documented in SRC-0071; ClasSICK does not need Apple's.
- Logical block 2 starts the **master directory block**, always two blocks:
  64 bytes of volume information, then the allocation block map (II-120–122).
- **Volume information** (II-121, Figure 7): signature word $D2D7, creation and
  backup dates, attributes (bit 7 hardware lock, bit 15 software lock), file
  count, first directory block, directory length in blocks, allocation-block
  count, allocation-block size, clump size, first allocation block in the map,
  next file number, free allocation blocks, and the volume name (length byte
  plus characters). Offsets are in the figure.
- **Allocation block map** (II-122): one 12-bit entry per allocation block,
  starting with block 2. 0 = unused, 1 = last block of a file, 2–4095 = next
  block. Directory blocks placed inside the allocation area are marked $FFF.
  **[OPEN]** How two 12-bit entries pack into three bytes is not stated; decide
  with synthetic fixtures and confirm by observation.
- **File directory** (II-122–123, Figure 8): one entry per file, 51 bytes plus
  the name, always a whole number of words, never crossing a logical block. Each
  entry has flags (bit 7 in use, bit 0 locked), version number, 16 bytes of
  Finder information, file number, then for each fork its first allocation
  block, logical length and physical length (start block 0 if the fork is
  absent), creation and modification dates (seconds since 1 January 1904), and
  the name. A 400K volume's directory occupies 12 logical blocks.
- **Finder information** (16 bytes, II-84, PDF 652): file type, creator, flags
  (bit 13 has bundle, bit 14 invisible), icon location and folder number.
  Applications have file type `'APPL'` (III-11, PDF 1014).
- **[DOC85]** All formats "may be different in future versions" (II-122–123).
  **[DOCPE]** The Promotional Edition's File Manager chapter (5/21/84, chapter
  pages 54–56, PDF 860–862) gives the same volume information, 12-bit block map
  and 51-byte directory entry.

**[ENG]** The existing `formats/mfs` placeholder fits here: a bounded,
read-only parser over generic 512-byte blocks, no native struct casts,
synthetic fixtures first, as its README already requires.

## 2. Resource file format

**[DOC85]** I-128–131 (PDF 138–141); **[DOCPE]** Resource Manager 11/28/84:

- A 16-byte header gives the offsets and lengths of the resource data and the
  resource map. Then 112 bytes and 128 bytes of application data. **[OPEN]**
  The 1985 text calls the 112 bytes "reserved for system use"; the 11/28/84 text
  calls them a partial copy of the file's directory entry used by the Finder.
  ClasSICK should neither require nor interpret them.
- **Resource data:** for each resource, a 4-byte length and the data.
- **Resource map:** 16 + 4 + 2 bytes reserved (filled in memory with a copy of
  the header, a next-map handle and the file reference number), 2 bytes of file
  attributes, then offsets to the type list and name list.
- **Type list:** count of types minus 1; for each type, the 4-byte type, count
  of resources minus 1, and offset to its reference list.
- **Reference list entry (12 bytes):** ID, offset to name (−1 if none), 1 byte of
  attributes, 3-byte offset to the data's length field, and 4 bytes reserved for
  a handle.
- **Name list:** length byte and characters per name.
- **Attributes** (I-111, PDF 121): resSysHeap 64, resPurgeable 32, resLocked 16,
  resProtected 8, resPreload 4, resChanged 2; bits 0 and 7 are not for
  applications. Locked overrides purgeable.
- **[INF]** The 3-byte data offset limits the resource data area to 16 MB.

**[ENG]** The `formats/resourcefork` placeholder: a bounded, read-only parser
returning (type, ID) → (attributes, data span, name), rejecting overlapping or
out-of-range offsets. The in-memory map the Resource Manager builds (with
handles in the reserved fields) is a separate, later contract.

## 3. Code segments

**[DOC85]** II-55, II-60–62 (PDF 623, 628–630); **[DOCPE]** Segment Loader
8/24/84, chapter pages 12–13 (PDF 788–789):

- An application's code is stored as `'CODE'` resources, one per segment; a
  segment is at most 32 KB (II-55). Segment names are not kept after linking.
- A **loaded** segment has been read by the Resource Manager and **locked**; an
  unloaded one is made relocatable and purgeable (II-55). The main segment is
  always loaded and locked (II-57).
- **Segment header** (first 4 bytes of each segment except 0): offset of its
  first jump-table entry from the start of the jump table, and its number of
  entries (II-61).
- **`'CODE'` 0** (II-61–62), 16 bytes then the jump table: "above A5" size
  (A5 to the upper end of application space), "below A5" size (application
  globals plus QuickDraw globals), jump-table length, offset from A5 to the jump
  table, then the jump table. The offset is 32 and "above A5" is 32 plus the
  table length "for all applications" (1985); the 8/24/84 text says "for most
  applications", and its "below A5" size omits the QuickDraw globals **[OPEN]**.
- **Jump-table entry, 8 bytes** (II-61, Figure 2):
  - *Unloaded:* routine offset within its segment (2 bytes); an instruction
    that pushes the segment number for LoadSeg (4 bytes); the `_LoadSeg` trap
    (2 bytes).
  - *Loaded:* segment number (2 bytes); an instruction that jumps to the
    routine's address (6 bytes).
  - Callers always execute the last six bytes; the effect depends on the state.
- **[INF]** The book gives sizes, not instructions. In the 68000 encodings, the
  natural 4-byte instruction that pushes a word constant is MOVE.W immediate to
  −(A7), and a 6-byte jump to an absolute address is JMP with an absolute long
  operand. The unloaded form is written by the original linker into
  `'CODE'` 0; the loaded form is written by the Segment Loader, so ClasSICK
  chooses its own 6-byte jump **[ENG]**.
- **[OPEN]** Whether the routine offset is measured from the segment's first
  byte or from just after its 4-byte header. Settle with synthetic fixtures and
  a reference observation.
- **[OPEN]** The instruction forms compiled code uses to call through the jump
  table are not documented. A5-relative addressing at offset 32 + 8n is the
  natural reading of II-62, not a stated rule.

## 4. Application memory: the A5 world

**[DOC85]** II-19–20 (PDF 587–588), II-57–64 (PDF 625–632):

- From high to low inside the application space: the jump table up to `BufPtr`;
  32 bytes of application parameters starting at A5 (the first is a pointer to
  the QuickDraw globals); application globals; 206 bytes of QuickDraw globals;
  then the stack growing down from `CurStackBase`, toward the application heap
  growing up from `ApplZone` (II-19, Figure 9).
- Register A5 and `CurrentA5` point to the boundary between the application
  globals and the application parameters (II-19–20; I-95).
- **[INF]** So A5 = `BufPtr` − "above A5" size, the jump table starts at
  A5 + `CurJTOffset` (32), and `CurStackBase` = A5 − "below A5" size.
- The Segment Loader reinitializes the application heap at each launch
  (`InitApplZone`, II-28, PDF 596) and keeps a desk-scrap handle across launches
  (II-60).
- `Launch` and `Chain` take a pointer to the file name at (A0) and a word at
  4(A0) choosing the sound and screen buffers: 0 main buffers, positive the
  alternate sound buffer, negative both alternate buffers (II-59–60). That word
  is kept in `CurPageOption`.
- **Globals** (Appendix D; II-64): `BufPtr` $10C, `CurApRefNum` $900,
  `CurrentA5` $904, `CurStackBase` $908, `CurApName` $910 (32 bytes),
  `CurJTOffset` $934, `CurPageOption` $936, `AppParmHandle` $AEC, `FinderName`
  $2E0.
- **Finder information** for the application (II-55–58): message word
  (0 open, 1 print), count word, then per document a volume reference number,
  file type, version, a spare byte and the name, padded to even length. Reached
  through `AppParmHandle`. **[DOCPE]** The 2/8/84 *Structure of a Macintosh
  Application* chapter describes the same document-passing scheme.
- **[OPEN]** Not documented: initial values of registers other than A5 and A7
  at entry, whether application globals are cleared, and what is on the stack
  when the main segment starts. These need reference observation; until then
  ClasSICK should choose and record deterministic values **[ENG]**.

## 5. Launch sequence

**[DOC85]** II-62: after reading the jump table from `'CODE'` 0, the Segment
Loader runs the first jump-table entry; that entry loads the main segment
(`'CODE'` 1) and starts the application.

**[ENG]** A ClasSICK launch therefore needs, in order:

1. Open the application's resource fork as the current resource file and record
   `CurApRefNum`, `CurApName` and `CurPageOption`.
2. Reinitialize the application heap.
3. Read `'CODE'` 0, validate its sizes against the space available, place the A5
   world, copy the jump table and set `CurrentA5`, `CurStackBase`,
   `CurJTOffset` and A5/A7.
4. Build the Finder information block and set `AppParmHandle`.
5. Start execution at the first jump-table entry's last six bytes.

## 6. Segment Loader traps

All six are **Toolbox-format** trap words (Appendix C, III-215–220):
LoadSeg A9F0 (III-218), UnloadSeg A9F1 (III-220), Launch A9F2 (III-218), Chain
A9F3 (III-215), ExitToShell A9F4 (III-216), GetAppParms A9F5 (III-216).

- **LoadSeg** (II-60): segment number as a word on the stack. Loads the segment
  if needed, sets all its jump-table entries to the loaded state, then invokes
  the routine that was called. **[INF]** Because the caller reached LoadSeg
  through an entry's last six bytes, "the routine that was called" is identified
  by that entry; how LoadSeg finds the entry (for example from its return
  address) is not documented **[OPEN]**.
- **UnloadSeg** (II-59): argument is the address of any routine in the segment;
  restores its entries to the unloaded state; the segment stays in memory until
  purged. It works only when called from outside the segment.
- **ExitToShell** (II-59): releases the application heap and launches the
  program named in `FinderName`.
- CountAppFiles, GetAppFiles and ClrAppFiles are not ROM traps (II-57–58); they
  read the Finder information.

## What this asks of the current code

Compared with SPEC-0020 (trap services) and SPEC-0022 (guest memory and
data/control instructions) at public revision `61c6c8d`:

| Need | Why | Status |
| --- | --- | --- |
| MOVE.W immediate to −(A7) | Unloaded jump-table entries | Covered by SPEC-0022's MOVE modes (verify the A7 case in tests) |
| Toolbox-format trap dispatch with stack arguments | `_LoadSeg` (A9F0) and the other Segment Loader traps | SPEC-0020 returns UNSUPPORTED_FORMAT for Toolbox words |
| JMP absolute long | Loaded jump-table entries | Not supported (SPEC-0022: "No JSR/JMP") |
| JSR and address-register-with-displacement addressing | Calls through the jump table, A5-relative globals | Not supported |
| LEA, PEA, LINK/UNLK, MOVEM, arithmetic, compare, shifts | Ordinary compiled code | Not supported |
| A guest RAM layout for the application space | A5 world, heap, stack, screen | SPEC-0022 maps regions but no Macintosh layout yet |
| Memory Manager phase 1 | Loading segments as locked handles; heap reset | Drafts on the owner's PC for the lead's review |
| Resource and MFS readers | Steps 1–3 | Placeholders only |

## Test plan: an application we build ourselves

No Apple application, disk image or resource is used. **[ENG]** Proposal:

1. **Generator tool** (project-owned, deterministic): assembles a few original
   68000 routines with the pinned GCC 13.3 cross tools (SRC-0081), emits a
   resource fork containing `'CODE'` 0 (two jump-table entries, unloaded) and
   `'CODE'` 1 and 2, and writes an MFS image whose one file has type `'APPL'`.
   The generator, not hand-edited bytes, is the provenance for every fixture.
2. **Fixture program:** `'CODE'` 1 stores a marker in its A5 globals, calls a
   routine in `'CODE'` 2 through the jump table (forcing LoadSeg), checks the
   returned value, calls one Memory Manager trap, and ends with ExitToShell.
3. **Hosted tests first:** MFS parser over the image (volume information, block
   chains, directory entries, both forks, corrupted-offset rejection); resource
   parser (type/ID lookup, attributes, names, out-of-range offsets); A5-world
   arithmetic from `'CODE'` 0; jump-table state changes on LoadSeg and UnloadSeg.
4. **Interpreter tests:** the fixture runs to ExitToShell in the bounded PC
   interpreter, with a fixed step budget and checked guest memory.
5. **Native and reference:** the same image on the PC build; then, under an
   authorized protocol, the OPEN items above on a January 1984 reference.

A real 1984 program will also call InitGraf, InitFonts, InitWindows, InitMenus,
TEInit, InitDialogs and InitCursor right after starting, so QuickDraw and the
other managers are the next wall after loading. The fixture deliberately avoids
them.

## Suggested order

1. Generator tool and synthetic MFS/resource fixtures.
2. Read-only MFS and resource-fork parsers (hosted C).
3. Memory Manager phase 1 (from the drafts).
4. CPU: JMP, JSR, displacement addressing, LEA/PEA, then general instructions.
5. Toolbox-format trap dispatch with stack arguments (SPEC-0017).
6. Segment Loader: launch, A5 world, LoadSeg/UnloadSeg, ExitToShell.
7. Run the fixture on the PC interpreter; then QuickDraw initialization for real
   programs.

## Open questions

| ID | Question |
| --- | --- |
| AL-1 | Byte packing of the MFS 12-bit allocation map entries |
| AL-2 | Whether a jump-table routine offset counts the 4-byte segment header |
| AL-3 | How LoadSeg identifies the entry that was called |
| AL-4 | Register, stack and globals state when the main segment starts |
| AL-5 | "Below A5" includes QuickDraw globals (1985) or not (8/24/84); jump-table offset always 32 (1985) or usually (8/24/84) |
| AL-6 | Meaning of the 112 reserved bytes in a resource file (1985 vs 11/28/84) |
| AL-7 | Whether January 1984 programs and the original Finder rely on any of the above differently |
