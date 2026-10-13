# First 68000 application: interface set — research record, 2026-10-13

Research record prepared by Claude at the owner's request. It is not a
specification and assigns no stable identifiers; the implementation lead
decides what to adopt. It changes no code.

Step 6 of the [68000 application runtime plan](../development/m68k-runtime-plan.md)
calls for executing an independently generated resource-fork application through
the managers. This record identifies the smallest set of Toolbox and Operating
System routines such a program needs, in two tiers:

- **Tier 1** opens one window, draws in it, follows the mouse and quits.
- **Tier 2** adds menus and desk-accessory support.

For each routine it gives the verified trap word, calling convention, stack
layout and memory-movement status. It also covers the documented initialization
order, the QuickDraw globals and A5 world, and the guest records and resources
involved. The same facts are in
[first-application-interfaces.json](first-application-interfaces.json),
shaped for the interface registry (plan step 1), alongside the typed guest-trap
work. Related: [application loading](application-loading.md).

## Sources, identity checks and labels

| Source | Identity verified 2026-10-13 | Page mapping and check |
| --- | --- | --- |
| **SRC-0071**, *Inside Macintosh* I–III (1985), owner's PDF | 19,245,372 bytes; SHA-256 `b65fe8ef…1d97`; 1,284 pages; matches the record | I = PDF − 10, II = PDF − 568, III = PDF − 1003. The printed footer of every cited page was checked. Appendix B (III-211–213) and Appendix C (III-215–220) were also read as page images. |
| **SRC-0068**, Promotional Edition (March 1985), local scan | 53,888,388 bytes; SHA-1 `4ed5b3c2…f13c5`; SHA-256 `92490207…b237`; 1,326 pages; matches | Each chapter is paged separately; citations use the edition's own index codes with the chapter's date (for example **WM-22 (5/30/84)**). The printed header of every cited page was checked. |

Labels: **[BOTH]** both editions agree; **[DOC85]** 1985 only; **[DIFF]**
editions disagree; **[INF]** inferred from documented facts; **[ENG]** ClasSICK
proposal; **[OPEN]** unresolved.

Neither edition is evidence of January 1984 behavior. One chapter predates it:
QuickDraw, dated 3/2/83. No sample code, equate file, Apple resource,
application, disk image, ROM or emulator source is an input. The manual's
example program (I-13 onward) is not used.

## 1. In short

1. **41 routines**: 23 in tier 1 and 18 in tier 2 (section 2).
   - All but one are Toolbox-format traps (bit 11 set) with the stack-based
     convention. *Inside Macintosh* gives register conventions wherever they
     exist, and none is given for these (I-90).
   - The exception is FlushEvents, an Operating System trap with D0 in and
     out (II-69).
2. **Every trap word was read from the page images** of Appendix C and agrees
   between its alphabetical and numeric tables.
   - Trap numbers follow III-215: the last two hex digits, or 1 followed by
     them when the word begins A9.
   - Three macro names differ from the Pascal names: `_GetRMenu` (GetMenu,
     I-351), `_DisposWindow` (DisposeWindow, I-284) and `_SysEdit` (SystemEdit,
     I-441).
3. **30 of the 41 may move or purge memory** (Appendix B, III-211–213), including
   FrameRect, EraseRect, DrawString and GetNextEvent. Eleven do not: InitGraf,
   InitCursor, FlushEvents, SetPort, MoveTo, FindWindow, FrontWindow,
   InitDialogs, GetItem, SystemTask and CloseDeskAcc. The guest-trap binding can
   carry this flag from the start.
4. **Initialization order is documented as a chain** (section 3): InitGraf →
   InitFonts → InitWindows → InitMenus → TEInit → InitDialogs. The Toolbox Event
   Manager needs InitWindows first (I-255). Desk-accessory support needs TEInit,
   InitDialogs, the Apple menu first and a standard Edit menu (I-439).
5. **QuickDraw globals** (section 4):
   - There are nine documented globals in 130 bytes **[INF]**, inside a
     206-byte area (`grafSize`, I-209).
   - They are stored from high to low memory below the pointer.
   - InitGraf stores the pointer to them at (A5), the first application
     parameter (I-163; II-20).
   - Where the area sits is up to the application's development system: I-163
     says most put it immediately below A5, while Figure 9 shows it below the
     application globals (II-19–20). This narrows open question AL-5 in
     application loading.
6. **The 3/2/83 QuickDraw chapter differs** **[DIFF]**:
   - globals at any location large enough (QD-34, QD-74);
   - routines called by JSR to linked code, not traps (QD-76);
   - `screenBits` bounds given as (0,0,512,342) (QD-34).

## 2. The routine set

**Tier 1 — one window**

| Routine | Trap macro and word | Number | Convention | Parameters and result | Stack bytes (I-91) | Moves memory (III-211–213) | 1985 | Promotional Edition | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| InitGraf | `_InitGraf` A86E (III-217) | $06E | Toolbox, stack | (globalPtr: Ptr) | 4 | No | I-162, I-163 | QD-34 (3/2/83) | Once only; pointer to thePort; stores it at (A5); globalPtr typed QDPtr in 3/2/83 |
| InitFonts | `_InitFonts` A8FE (III-217, III-224) | $0FE | Toolbox, stack | — | 0 | Yes | I-222 | FM-9 (6/11/84) | After QuickDraw, before the Window Manager. Alphabetical entry smudged; numeric table reads A8FE |
| InitWindows | `_InitWindows` A912 (III-217) | $112 | Toolbox, stack | — | 0 | Yes | I-280, I-281 | WM-20 (5/30/84) | Draws desktop and empty menu bar |
| InitCursor | `_InitCursor` A850 (III-217) | $050 | Toolbox, stack | — | 0 | No | I-167 | QD-39 (3/2/83) | Arrow cursor, level 0 |
| FlushEvents | `_FlushEvents` A032 (III-216) | $032 | OS register | D0 low word = eventMask, D0 high word = stopMask; exit D0 = 0 or event code (word) | — | No | II-69 | OE-5 (11/19/84) | The only register-based routine in the set |
| GetNewWindow | `_GetNewWindow` A9BD (III-217) | $1BD | Toolbox, stack | (windowID: INTEGER; wStorage: Ptr; behind: WindowPtr) : WindowPtr | 10 + 4 result | Yes | I-283, I-302 | WM-22 (5/30/84) | Reads a WIND template |
| NewWindow | `_NewWindow` A913 (III-218) | $113 | Toolbox, stack | (wStorage: Ptr; boundsRect: Rect; title: Str255; visible: BOOLEAN; procID: INTEGER; behind: WindowPtr; goAwayFlag: BOOLEAN; refCon: LONGINT) : WindowPtr | 26 + 4 result | Yes | I-282 | WM-21 (5/30/84) |  |
| SetPort | `_SetPort` A873 (III-220) | $073 | Toolbox, stack | (port: GrafPtr) | 4 | No | I-165 | QD-36 (3/2/83) | Only SetPort, OpenPort and InitPort change the current port; parameter named gp in 3/2/83 |
| BeginUpdate | `_BeginUpdate` A922 (III-215) | $122 | Toolbox, stack | (theWindow: WindowPtr) | 4 | Yes | I-292 | WM-32 (5/30/84) |  |
| EndUpdate | `_EndUpdate` A923 (III-216) | $123 | Toolbox, stack | (theWindow: WindowPtr) | 4 | Yes | I-293 | WM-32 (5/30/84) |  |
| EraseRect | `_EraseRect` A8A3 (III-216) | $0A3 | Toolbox, stack | (r: Rect) | 4 | Yes | I-177 | QD-49 (3/2/83) |  |
| MoveTo | `_MoveTo` A893 (III-218) | $093 | Toolbox, stack | (h: INTEGER; v: INTEGER) | 4 | No | I-170 | QD-42 (3/2/83) |  |
| DrawString | `_DrawString` A884 (III-216) | $084 | Toolbox, stack | (s: Str255) | 4 | Yes | I-172 | QD-44 (3/2/83) |  |
| FrameRect | `_FrameRect` A8A1 (III-216) | $0A1 | Toolbox, stack | (r: Rect) | 4 | Yes | I-176 | QD-49 (3/2/83) |  |
| GetNextEvent | `_GetNextEvent` A970 (III-217) | $170 | Toolbox, stack | (eventMask: INTEGER; VAR theEvent: EventRecord) : BOOLEAN | 6 + 2 result | Yes | I-257, I-258 | EM-21 (11/19/84) | FALSE for a null event or one the Desk Manager handled (SystemEvent) |
| FindWindow | `_FindWindow` A92C (III-216) | $12C | Toolbox, stack | (thePt: Point; VAR whichWindow: WindowPtr) : INTEGER | 8 + 2 result | No | I-287 | WM-26 (5/30/84) | Returns a part code |
| DragWindow | `_DragWindow` A925 (III-216) | $125 | Toolbox, stack | (theWindow: WindowPtr; startPt: Point; boundsRect: Rect) | 12 | Yes | I-289 | WM-28 (5/30/84) |  |
| SelectWindow | `_SelectWindow` A91F (III-219) | $11F | Toolbox, stack | (theWindow: WindowPtr) | 4 | Yes | I-284 | WM-23 (5/30/84) |  |
| TrackGoAway | `_TrackGoAway` A91E (III-220) | $11E | Toolbox, stack | (theWindow: WindowPtr; thePt: Point) : BOOLEAN | 8 + 2 result | Yes | I-288 | WM-27 (5/30/84) |  |
| CloseWindow | `_CloseWindow` A92D (III-215) | $12D | Toolbox, stack | (theWindow: WindowPtr) | 4 | Yes | I-283 | WM-22 (5/30/84) | For caller-supplied storage |
| DisposeWindow | `_DisposWindow` A914 (III-216) | $114 | Toolbox, stack | (theWindow: WindowPtr) | 4 | Yes | I-284 | WM-23 (5/30/84) | Macro name _DisposWindow |
| FrontWindow | `_FrontWindow` A924 (III-216) | $124 | Toolbox, stack | → WindowPtr | 0 + 4 result | No | I-286 | WM-26 (5/30/84) |  |
| ExitToShell | `_ExitToShell` A9F4 (III-216) | $1F4 | Toolbox, stack | — | 0 | Yes | II-59 | SL-9 (8/24/84) | Does not return; launches the application named in FinderName |

**Tier 2 — menus and desk accessories**

| Routine | Trap macro and word | Number | Convention | Parameters and result | Stack bytes (I-91) | Moves memory (III-211–213) | 1985 | Promotional Edition | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| InitMenus | `_InitMenus` A930 (III-217) | $130 | Toolbox, stack | — | 0 | Yes | I-349, I-351 | MN-15 (9/24/84) | Once only |
| TEInit | `_TEInit` A9CC (III-220) | $1CC | Toolbox, stack | — | 0 | Yes | I-381, I-383 | TE-16 (1/14/85) | Call even without TextEdit, for desk accessories and dialogs |
| InitDialogs | `_InitDialogs` A97B (III-217) | $17B | Toolbox, stack | (resumeProc: ProcPtr) | 4 | No | I-410, I-411 | DL-18 (7/6/84) | Parameter called restartProc, a fail-safe procedure, in 7/6/84 |
| NewMenu | `_NewMenu` A931 (III-218) | $131 | Toolbox, stack | (menuID: INTEGER; menuTitle: Str255) : MenuHandle | 6 + 4 result | Yes | I-351 | MN-16 (9/24/84) |  |
| GetMenu | `_GetRMenu` A9BF (III-217) | $1BF | Toolbox, stack | (resourceID: INTEGER) : MenuHandle | 2 + 4 result | Yes | I-351 | MN-16, MN-17 (9/24/84) | Macro name _GetRMenu |
| AppendMenu | `_AppendMenu` A933 (III-215) | $133 | Toolbox, stack | (theMenu: MenuHandle; data: Str255) | 8 | Yes | I-352 | MN-17 (9/24/84) |  |
| AddResMenu | `_AddResMenu` A94D (III-215) | $14D | Toolbox, stack | (theMenu: MenuHandle; theType: ResType) | 8 | Yes | I-353 | MN-18 (9/24/84) | ResType is a 4-byte array, so passed by contents (I-91) |
| InsertMenu | `_InsertMenu` A935 (III-217) | $135 | Toolbox, stack | (theMenu: MenuHandle; beforeID: INTEGER) | 6 | Yes | I-353 | MN-19 (9/24/84) |  |
| DrawMenuBar | `_DrawMenuBar` A937 (III-216) | $137 | Toolbox, stack | — | 0 | Yes | I-354 | MN-19 (9/24/84) |  |
| MenuSelect | `_MenuSelect` A93D (III-218) | $13D | Toolbox, stack | (startPt: Point) : LONGINT | 4 + 4 result | Yes | I-355 | MN-21 (9/24/84) | Menu ID in high word, item in low word |
| MenuKey | `_MenuKey` A93E (III-218) | $13E | Toolbox, stack | (ch: CHAR) : LONGINT | 2 + 4 result | Yes | I-356 | MN-22 (9/24/84) |  |
| HiliteMenu | `_HiliteMenu` A938 (III-217) | $138 | Toolbox, stack | (menuID: INTEGER) | 2 | Yes | I-357 | MN-22 (9/24/84) |  |
| GetItem | `_GetItem` A946 (III-217) | $146 | Toolbox, stack | (theMenu: MenuHandle; item: INTEGER; VAR itemString: Str255) | 10 | No | I-358 | MN-23 (9/24/84) |  |
| OpenDeskAcc | `_OpenDeskAcc` A9B6 (III-218) | $1B6 | Toolbox, stack | (theAcc: Str255) : INTEGER | 4 + 2 result | Yes | I-440 | DS-7 (8/22/84) | Reads the accessory into the application heap; result should be ignored |
| CloseDeskAcc | `_CloseDeskAcc` A9B7 (III-215) | $1B7 | Toolbox, stack | (refNum: INTEGER) | 2 | No | I-440 | DS-8 (8/22/84) | refNum from the window's windowKind |
| SystemClick | `_SystemClick` A9B3 (III-220) | $1B3 | Toolbox, stack | (theEvent: EventRecord; theWindow: WindowPtr) | 8 | Yes | I-441 | DS-8 (8/22/84) | EventRecord by value is passed as a pointer (I-91) |
| SystemEdit | `_SysEdit` A9C2 (III-220) | $1C2 | Toolbox, stack | (editCmd: INTEGER) : BOOLEAN | 2 + 2 result | Yes | I-441, I-442 | DS-9 (8/22/84) | Macro name _SysEdit; editCmd = item number - 1 |
| SystemTask | `_SystemTask` A9B4 (III-220) | $1B4 | Toolbox, stack | — | 0 | No | I-442 | DS-10 (8/22/84) | Runs desk accessories' periodic actions; not listed in Appendix B |

Stack bytes count the parameters a caller pushes, plus result space reserved
before them, using the I-91 sizes. Every routine removes its own parameters
(I-91; AL-10).

## 3. Initialization order

| Rule | 1985 | Promotional Edition |
| --- | --- | --- |
| InitGraf once only, at the start | I-162 | QD-34 (3/2/83) [BOTH] |
| InitFonts after QuickDraw, before the Window Manager | I-222 | FM-9 (6/11/84) [BOTH] |
| Window Manager: InitGraf and InitFonts first; then InitWindows | I-280 | WM-18 (5/30/84) [BOTH] |
| Toolbox Event Manager: initialize the Window Manager first; that needs QuickDraw and the Font Manager | I-255 | Not checked |
| Menu Manager: InitGraf, InitFonts, InitWindows; then InitMenus | I-349 | MN-13 (9/24/84) [BOTH] |
| TextEdit: QuickDraw, Font Manager, Window Manager in that order; then TEInit, even without TextEdit, so that desk accessories and dialog and alert boxes work | I-381, I-383 | TE-14, TE-16 (1/14/85) [BOTH] |
| Dialog Manager: QuickDraw, Font Manager, Window Manager, Menu Manager, TextEdit in that order; then InitDialogs | I-410 | DL-17 (7/6/84) [BOTH] |
| Desk accessories: TEInit and InitDialogs; Apple menu first; Edit menu with Undo, Cut, Copy, Paste, Clear; GetItem then OpenDeskAcc; CloseDeskAcc on Close for a system window | I-439–440 | DS-7–8 (8/22/84) |
| Before drawing in a window, make it the current port with SetPort | I-280 | Not checked |

**[INF]** A sequence satisfying every rule: InitGraf, InitFonts, InitWindows,
InitMenus, TEInit, InitDialogs, InitCursor, with FlushEvents wherever the
application wants a clean queue. The manuals place InitCursor and FlushEvents
in no order.

## 4. QuickDraw globals and the A5 world

**[BOTH]** InitGraf initializes these globals (I-162–163; QD-34):

| Global | Type | Initial value | Size **[INF]** |
| --- | --- | --- | --- |
| thePort | GrafPtr | NIL | 4 |
| white, black, gray, ltGray, dkGray | Pattern (I-146) | all white, all black, 50%, 25%, 75% gray | 8 each, 40 total |
| arrow | Cursor (I-146) | the standard arrow | 68 (two 16-word arrays and a Point) |
| screenBits | BitMap (I-144) | the entire screen (1985); "Macintosh screen, (0,0,512,342)" (3/2/83) | 14 |
| randSeed | LONGINT | 1 | 4 |

- The nine documented globals total 130 bytes **[INF]**. `grafSize` is 206
  (I-209), and Figure 9 shows a 206-byte QuickDraw globals area (II-19). InitGraf
  also sets private globals for its own use (I-162). **[INF]** About 76 bytes
  are private, and their contents are undocumented.
- **[DOC85]** The globals are stored in reverse order, from high to low memory.
  Most development systems preallocate them immediately below the location A5
  points to. InitGraf stores the pointer to `thePort` in the location pointed
  to by A5, and the other globals are reached at negative offsets from that
  pointer (I-163).
- **[DOC85]** The 32 bytes of application parameters lie above the application
  globals. The first is the address of the first QuickDraw global. Some
  development systems may place the globals elsewhere, but the first parameter
  always points to them, so the location A5 points to always holds that
  address (II-20).
- **[DOC85]** Figure 9 (II-19) shows one arrangement: from high memory, the jump
  table, the application parameters with A5 at their base, the application
  globals, then the 206-byte QuickDraw globals ending at `CurStackBase`, then
  the stack. That differs from I-163's "immediately below" A5; II-20 explains
  that placement varies by development system.
- **[DIFF]** The 3/2/83 chapter says assembly-language programmers may put the
  globals anywhere that holds `GRAFSIZE` bytes (QD-34, QD-74). It describes
  calling QuickDraw by pushing parameters and executing JSR to the routine
  (QD-76). Its example reaches the globals through a named offset from A5;
  this record draws no rule from an example.
- **[INF] Consequence for the launch work.** The QuickDraw globals live in space
  the application's development system lays out. The system's launch duty is A5,
  `CurrentA5` and the 32-byte application parameters. InitGraf, called by the
  program, fills the first parameter. This supports reading `'CODE'` 0's
  below-A5 size as including the globals when the development system put them
  there. AL-5 remains open for January 1984.

## 5. Guest records and resources the set touches

| Record or resource | Size or format | 1985 | Promotional Edition |
| --- | --- | --- | --- |
| Point | v, h: two INTEGERs, 4 bytes **[INF]** | I-139 | — |
| Rect | top, left, bottom, right: four INTEGERs, 8 bytes **[INF]** | I-141 | — |
| EventRecord | what (2), message (4), when (4), where (4), modifiers (2): 16 bytes **[INF]** | I-249 | EM-11 (11/19/84) [BOTH] |
| BitMap | baseAddr (4), rowBytes (2), bounds (8): 14 bytes **[INF]** | I-144 | — |
| Pattern, Cursor | 8 bytes; 68 bytes **[INF]** | I-146 | — |
| GrafPort | Fields listed; applications read `portRect` | I-148 | — |
| WindowRecord | Fields listed; `windowKind` gives a desk accessory's reference number for CloseDeskAcc | I-276, I-440 | — |
| MenuInfo | Fields listed | I-345 | — |
| `'WIND'` template | boundsRect 8, procID 2, visible 2, goAwayFlag 2, refCon 4, then the title as length byte and characters | I-302 | WM-42–43 (5/30/84) [BOTH] |
| `'MENU'` resource | Menu ID, placeholders, definition procedure ID, enable flags, title, items | I-364 | — |

FindWindow returns one of a set of documented part codes (I-287).

## 6. What the guest-trap binding has to handle [ENG]

The set exercises every I-91 parameter form except floating point:

- **Rect** (8 bytes) by address: EraseRect, FrameRect, DragWindow, NewWindow.
- **Point** (4 bytes) by contents: FindWindow, DragWindow, TrackGoAway,
  MenuSelect.
- **Str255** by address, in and out: DrawString, NewMenu, AppendMenu,
  OpenDeskAcc, GetItem (VAR).
- **EventRecord**: by address as VAR (GetNextEvent), and by value, which I-91
  also passes by address (SystemClick).
- **BOOLEAN**: in (NewWindow) and as a result (GetNextEvent, TrackGoAway,
  SystemEdit), with the value in bit 0 of the high-order byte.
- **CHAR** with the value in the low-order byte (MenuKey).
- **LONGINT** results with packed halves: MenuSelect and MenuKey return the menu
  ID in the high word and the item in the low word (I-355–356).
- **ResType**: a 4-byte array, so passed by contents (AddResMenu).
- **ProcPtr**: InitDialogs' resume procedure (entry A9 of Claude's separate
  callback catalog draft, if the lead adopts it).
- **Routines that keep a pointer:** InitGraf (the globals), GetNewWindow and
  NewWindow (`wStorage`, `behind`).
- **A routine that never returns:** ExitToShell (II-59).

Proposed order **[ENG]**: bind tier 1 first, then tier 2. Give every record the
may-move-memory flag from section 2 and the init-order preconditions from
section 3. Keep the guest record layouts in section 5 in the registry's
`guest_records`.

## 7. Edition differences

| Topic | 1985 (SRC-0071) | Promotional Edition (SRC-0068) |
| --- | --- | --- |
| QuickDraw calling mechanism | Trap macros (Appendix C; I-88–90) | Push parameters and JSR to the routine (QD-76, 3/2/83) |
| QuickDraw globals' location | Below A5, reverse order; pointer at (A5) (I-163; II-20) | Any location of `GRAFSIZE` bytes (QD-34, QD-74) |
| `screenBits` initial bounds | "The entire screen" (I-163) | (0,0,512,342) (QD-34) |
| InitGraf parameter type | Ptr (I-162) | QDPtr (QD-34) |
| SetPort parameter name | port (I-165) | gp (QD-36) |
| InitDialogs parameter | resumeProc, a resume procedure (I-411) | restartProc, a "fail-safe procedure" (DL-18) |

The other routines in the set have the same parameters and results in both
editions, at the Promotional Edition pages in section 2.

## 8. A proposed original test application [ENG]

An independently written program, with its own generated resources, to exercise
exactly this set:

- **Tier 1:**
  - `'CODE'` 0 and 1;
  - one `'WIND'` (an original title, document window, no close box at first);
  - the init sequence from section 3;
  - draw an original pattern of rectangles and a string in the update handler;
  - drag and select windows;
  - quit on a mouse click or after a fixed number of events.
- **Tier 2:**
  - add `'MENU'` resources for an Apple menu filled by AddResMenu with `'DRVR'`;
  - a File menu with Quit, and a standard Edit menu;
  - handle MenuSelect and MenuKey;
  - route system windows through SystemClick, SystemEdit and CloseDeskAcc.

Desk accessories need `'DRVR'` code. Running Apple's is outside the clean-room
scope, so tier 2 needs either an original test accessory or a run with the
Apple menu empty.

Nothing here copies the manual's example program (I-13 onward); its structure
and resource IDs are not inputs.

## 9. Decisions for the implementation lead

| ID | Decision | Recommendation |
| --- | --- | --- |
| FA-1 | Adopt this set as the target for the first original application | Yes, tier 1 first |
| FA-2 | How the system treats the 76 private QuickDraw global bytes | ClasSICK's own layout. Programs should not read them; document as unsupported |
| FA-3 | Whether the launch reserves QuickDraw globals itself, or trusts `'CODE'` 0's below-A5 size | Trust the program's sizes. Record, don't refuse, a below-A5 size smaller than `grafSize`, because placement is the development system's choice (I-163; II-20) |
| FA-4 | Initial `screenBits` bounds on larger PC screens | `system1-1984`: 512 × 342. Larger only under `classick-extended` |
| FA-5 | Tier 2 desk-accessory testing | An original test `'DRVR'`; never Apple's |

## 10. Open questions

| ID | Question |
| --- | --- |
| FQ-1 | Which of these traps the January 1984 System and ROM implement (SPEC-0017 Q7). All appear in the 1985 Appendix C and in Promotional Edition chapters dated 1983–1985 |
| FQ-2 | Contents of the private QuickDraw globals |
| FQ-3 | Whether January 1984 programs located QuickDraw globals as the 1985 text describes, or anywhere, as 3/2/83 allows |
| FQ-4 | Byte position of the 2-byte BOOLEAN fields in `'WIND'` (visible, goAwayFlag); the template table does not say |
| FQ-5 | Why SystemTask is absent from Appendix B although it runs desk-accessory code. The manual lists what it lists |

## 11. Citation verification

Each printed page below was read from the verified PDF. Its footer (SRC-0071)
or header (SRC-0068) matched.

**SRC-0071**, with PDF pages in parentheses:

- I-13 (23; only to locate the excluded example program), I-88–91 (98–101),
  I-139 (149), I-141 (151), I-144 (154), I-146 (156),
  I-148 (158)
- I-162–163 (172–173), I-165 (175), I-167 (177), I-170 (180), I-172 (182),
  I-176–177 (186–187), I-209 (219), I-222 (232)
- I-249 (259), I-255 (265), I-257–258 (267–268), I-276 (286)
- I-280–284 (290–294), I-286–289 (296–299), I-292–293 (302–303), I-302 (312)
- I-345 (355), I-349 (359), I-351–358 (361–368), I-364 (374)
- I-381 (391), I-383 (393), I-410–411 (420–421), I-439–442 (449–452)
- II-19–20 (587–588), II-59 (627), II-69 (637)
- III-211–213 (1214–1216, images), III-215–220 (1218–1223; III-215, 216, 217,
  218, 219 and 220 as images), III-224 (1227)

**SRC-0068**, with PDF pages in parentheses:

- AL-10 (108)
- QD-34 (288), QD-36 (290), QD-39 (293), QD-42 (296), QD-44 (298), QD-49
  (303), QD-74 (328), QD-76 (330)
- FM-9 (352), EM-11 (386), EM-21 (396)
- WM-18 (426), WM-20–23 (428–431), WM-26–28 (434–436), WM-32 (440), WM-42–43
  (450–451)
- MN-13 (509), MN-15–19 (511–515), MN-21–23 (517–519)
- TE-14 (546), TE-16 (548)
- DL-17–18 (581–582)
- DS-7–10 (615–618)
- SL-9 (785), OE-5 (797)

Claude (Anthropic; session configured as `claude-opus-5-5`) prepared this
record; human provenance review pending.
