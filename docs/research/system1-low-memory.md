# System 1 low-memory map, draft — 2026-10-11

Research draft for R-004 / R97-Q-04, prepared by Claude. It is not a
specification and accepts no 1984 behavior. Source: the owner's copy of
*Inside Macintosh* Volumes I–III (Addison-Wesley, 1985), SRC-0071, which
describes ROM version 105 in the Macintosh 128K and 512K and the System file
dated May 2, 1984 (I-4). Every row needs confirmation for the January 1984
System 1 profile (R-001) before a specification adopts it.

## Regions

| Range (128K) | Use | Source |
| --- | --- | --- |
| $000–$0FF | MC68000 exception vectors (256 bytes) | III-17 |
| $000–$AFF | System globals: the lowest 2816 bytes, including the vectors | II-19–20 |
| From the address in `SysZone` | System heap | II-19, Figure 9 |
| `ApplZone` to `HeapEnd` / `ApplLimit` | Application heap, growing up | II-19, Figure 9 |
| Below `CurStackBase` | Stack, growing down toward the heap | II-19, Figure 9 |
| `CurStackBase` up to A5 | QuickDraw and application globals | II-19–20 |
| A5 (`CurrentA5`) + 32 bytes | Application parameters; the first points to the QuickDraw globals | II-20 |
| Above the parameters, up to `BufPtr` | Jump table | II-19–20 |
| $12700 | Alternate screen buffer, 21,888 bytes, if requested | III-19 |
| $1A100 | Alternate sound buffer, 370 words, if requested | III-21 |
| $1A700 | Main screen buffer, 21,888 bytes | III-19 |
| $1FD00 | Main sound buffer, 370 words; low bytes drive disk speed | III-20–21 |
| Not stated | Trap dispatch table: 1024 bytes in RAM, 512 one-word entries | I-87–88 |

Figure 9 also reserves 128 bytes between the main screen and sound buffers for
the System Error Handler and leaves gaps whose sizes it states; the 512K adds
$60000 to each buffer address (III-19, III-21). Volumes I–III do not state the
trap dispatch table's address. Appendix D documents no global from $400 through
$823, a gap large enough for the 1024-byte table; that is consistent with a
table at $400 but is this project's inference, not a published statement. The
address stays open until a published source or owner observation supplies it.

## Global variables by address

Appendix D (III-227–231) lists 138 documented globals alphabetically with their
locations. The table below re-sorts them by address. Names and addresses were
checked against the page images, not only the OCR text. Unmarked sizes are the
appendix's own (byte, word, long or a stated count). `*` marks a size the
appendix does not state: this draft takes 4 bytes because the entry is described
as a pointer, handle, address or jump vector, and the book gives `Ptr` and
`Handle` as 4 bytes (I-86) and defines `ProcPtr` as `Ptr` (I-78). Those sizes
are this project's inference; the SCC/VIA rows below show that one can fail.
The grouping column is this project's own, not Apple's. Descriptions are
deliberately omitted: read the cited page.

| Address | Name | Bytes | Group | Page |
| --- | --- | --- | --- | --- |
| $102 | ScrVRes | 2 | Hardware and screen | III-230 |
| $104 | ScrHRes | 2 | Hardware and screen | III-230 |
| $108 | MemTop | 4* | Memory and application layout | III-229 |
| $10C | BufPtr | 4* | Memory and application layout | III-227 |
| $114 | HeapEnd | 4* | Memory and application layout | III-228 |
| $118 | TheZone | 4* | Memory and application layout | III-231 |
| $11C | UTableBase | 4* | Files, volumes and devices | III-231 |
| $130 | ApplLimit | 4* | Memory and application layout | III-227 |
| $144 | SysEvtMask | 2 | Events, input and time | III-230 |
| $14A | EventQueue | 10 | Events, input and time | III-228 |
| $156 | RndSeed | 4 | Constants and scratch | III-229 |
| $15C | SEvtEnb | 1 | Events, input and time | III-230 |
| $160 | VBLQueue | 10 | Events, input and time | III-231 |
| $16A | Ticks | 4 | Events, input and time | III-231 |
| $18E | KeyThresh | 2 | Events, input and time | III-228 |
| $190 | KeyRepThresh | 2 | Events, input and time | III-228 |
| $192 | Lvl1DT | 32 | Hardware and screen | III-228 |
| $1B2 | Lvl2DT | 32 | Hardware and screen | III-229 |
| $1D8 | SCCRd | 4* | Hardware and screen | III-229 |
| $1DA | VIA | 4* | Hardware and screen | III-231 |
| $1DC | SCCWr | 4* | Hardware and screen | III-229 |
| $1E4 | Scratch20 | 20 | Constants and scratch | III-230 |
| $1F8 | SPValid | 1 | Parameter RAM copy | III-230 |
| $1F8 | SysParam | 20 | Parameter RAM copy | III-230 |
| $1F9 | SPATalkA | 1 | Parameter RAM copy | III-230 |
| $1FA | SPATalkB | 1 | Parameter RAM copy | III-230 |
| $1FB | SPConfig | 1 | Parameter RAM copy | III-230 |
| $1FC | SPPortA | 2 | Parameter RAM copy | III-230 |
| $1FE | SPPortB | 2 | Parameter RAM copy | III-230 |
| $200 | SPAlarm | 4 | Parameter RAM copy | III-230 |
| $204 | SPFont | 2 | Parameter RAM copy | III-230 |
| $206 | SPKbd | 1 | Parameter RAM copy | III-230 |
| $207 | SPPrint | 1 | Parameter RAM copy | III-230 |
| $208 | SPVolCtl | 1 | Parameter RAM copy | III-230 |
| $209 | SPClikCaret | 1 | Parameter RAM copy | III-230 |
| $20B | SPMisc2 | 1 | Parameter RAM copy | III-230 |
| $20C | Time | 4 | Events, input and time | III-231 |
| $214 | SFSaveDisk | 2 | Files, volumes and devices | III-230 |
| $260 | SdVolume | 1 | Sound | III-230 |
| $262 | SoundPtr | 4* | Sound | III-230 |
| $266 | SoundBase | 4* | Sound | III-230 |
| $27F | SoundLevel | 1 | Sound | III-230 |
| $280 | CurPitch | 2 | Sound | III-227 |
| $291 | PortBUse | 1 | Files, volumes and devices | III-229 |
| $2A6 | SysZone | 4* | Memory and application layout | III-230 |
| $2AA | ApplZone | 4* | Memory and application layout | III-227 |
| $2AE | ROMBase | 4* | Traps, ROM and RAM bases | III-229 |
| $2B2 | RAMBase | 4* | Traps, ROM and RAM bases | III-229 |
| $2BA | DSAlertTab | 4* | System errors | III-228 |
| $2BE | ExtStsDT | 16 | Hardware and screen | III-228 |
| $2D8 | ABusVars | 4* | AppleTalk (post-System 1 feature) | III-227 |
| $2E0 | FinderName | 16 | Memory and application layout | III-228 |
| $2F0 | DoubleTime | 4 | Events, input and time | III-228 |
| $2F4 | CaretTime | 4 | Events, input and time | III-227 |
| $2F8 | ScrDmpEnb | 1 | Events, input and time | III-230 |
| $2FC | BufTgFNum | 4 | Files, volumes and devices | III-227 |
| $300 | BufTgFFlg | 2 | Files, volumes and devices | III-227 |
| $302 | BufTgFBkNum | 2 | Files, volumes and devices | III-227 |
| $304 | BufTgDate | 4 | Files, volumes and devices | III-227 |
| $308 | DrvQHdr | 10 | Files, volumes and devices | III-228 |
| $31A | Lo3Bytes | 4 | Constants and scratch | III-228 |
| $31E | MinStack | 4 | Memory and application layout | III-229 |
| $322 | DefltStack | 4 | Memory and application layout | III-228 |
| $328 | GZRootHnd | 4* | Memory and application layout | III-228 |
| $34E | FCBSPtr | 4* | Files, volumes and devices | III-228 |
| $352 | DefVCBPtr | 4* | Files, volumes and devices | III-228 |
| $356 | VCBQHdr | 10 | Files, volumes and devices | III-231 |
| $360 | FSQHdr | 10 | Files, volumes and devices | III-228 |
| $3F2 | ToExtFS | 4* | Files, volumes and devices | III-231 |
| $3F8 | DSAlertRect | 8 | System errors | III-228 |
| $824 | ScrnBase | 4* | Hardware and screen | III-230 |
| $8DE | JournalFlag | 2 | Events, input and time | III-228 |
| $8E8 | JournalRef | 2 | Events, input and time | III-228 |
| $8EC | CrsrThresh | 2 | Events, input and time | III-227 |
| $8F4 | JFetch | 4* | Files, volumes and devices | III-228 |
| $8F8 | JStash | 4* | Files, volumes and devices | III-228 |
| $8FC | JIODone | 4* | Files, volumes and devices | III-228 |
| $900 | CurApRefNum | 2 | Memory and application layout | III-227 |
| $904 | CurrentA5 | 4* | Memory and application layout | III-227 |
| $908 | CurStackBase | 4* | Memory and application layout | III-227 |
| $910 | CurApName | 32 | Memory and application layout | III-227 |
| $934 | CurJTOffset | 2 | Memory and application layout | III-227 |
| $936 | CurPageOption | 2 | Memory and application layout | III-227 |
| $944 | PrintErr | 2 | Printing | III-229 |
| $960 | ScrapSize | 4 | Scrap and TextEdit | III-229 |
| $964 | ScrapHandle | 4* | Scrap and TextEdit | III-229 |
| $968 | ScrapCount | 2 | Scrap and TextEdit | III-229 |
| $96A | ScrapState | 2 | Scrap and TextEdit | III-229 |
| $96C | ScrapName | 4* | Scrap and TextEdit | III-229 |
| $980 | ROMFont0 | 4* | Fonts | III-229 |
| $984 | ApFontID | 2 | Fonts | III-227 |
| $9CE | ToolScratch | 8 | Constants and scratch | III-231 |
| $9D6 | WindowList | 4* | Windows and desktop | III-231 |
| $9DA | SaveUpdate | 2 | Windows and desktop | III-229 |
| $9DC | PaintWhite | 2 | Windows and desktop | III-229 |
| $9DE | WMgrPort | 4* | Windows and desktop | III-231 |
| $9E6 | OldStructure | 4* | Windows and desktop | III-229 |
| $9EA | OldContent | 4* | Windows and desktop | III-229 |
| $9EE | GrayRgn | 4* | Windows and desktop | III-228 |
| $9F2 | SaveVisRgn | 4* | Windows and desktop | III-229 |
| $9F6 | DragHook | 4* | Windows and desktop | III-228 |
| $9FA | Scratch8 | 8 | Constants and scratch | III-230 |
| $A02 | OneOne | 4 | Constants and scratch | III-229 |
| $A06 | MinusOne | 4 | Constants and scratch | III-229 |
| $A1C | MenuList | 4* | Menus | III-229 |
| $A20 | MBarEnable | 2 | Menus | III-229 |
| $A24 | MenuFlash | 2 | Menus | III-229 |
| $A26 | TheMenu | 2 | Menus | III-231 |
| $A2C | MBarHook | 4* | Menus | III-229 |
| $A30 | MenuHook | 4* | Menus | III-229 |
| $A34 | DragPattern | 8 | Windows and desktop | III-228 |
| $A3C | DeskPattern | 8 | Windows and desktop | III-228 |
| $A50 | TopMapHndl | 4* | Resources | III-231 |
| $A54 | SysMapHndl | 4* | Resources | III-230 |
| $A58 | SysMap | 2 | Resources | III-230 |
| $A5A | CurMap | 2 | Resources | III-227 |
| $A5E | ResLoad | 2 | Resources | III-229 |
| $A60 | ResErr | 2 | Resources | III-229 |
| $A63 | FScaleDisable | 1 | Fonts | III-228 |
| $A64 | CurActivate | 4* | Windows and desktop | III-227 |
| $A68 | CurDeactive | 4* | Windows and desktop | III-227 |
| $A6C | DeskHook | 4* | Windows and desktop | III-228 |
| $A70 | TEDoText | 4* | Scrap and TextEdit | III-230 |
| $A74 | TERecal | 4* | Scrap and TextEdit | III-231 |
| $A78 | ApplScratch | 12 | Constants and scratch | III-227 |
| $A84 | GhostWindow | 4* | Windows and desktop | III-228 |
| $A8C | ResumeProc | 4* | Dialogs and alerts | III-229 |
| $A98 | ANumber | 2 | Dialogs and alerts | III-227 |
| $A9A | ACount | 2 | Dialogs and alerts | III-227 |
| $A9C | DABeeper | 4* | Sound | III-227 |
| $AA0 | DAStrings | 16 | Dialogs and alerts | III-227 |
| $AB0 | TEScrpLength | 4 | Scrap and TextEdit | III-231 |
| $AB4 | TEScrpHandle | 4* | Scrap and TextEdit | III-231 |
| $AD8 | SysResName | 20 | Resources | III-230 |
| $AEC | AppParmHandle | 4* | Memory and application layout | III-227 |
| $AF0 | DSErrCode | 2 | System errors | III-228 |
| $AF2 | ResErrProc | 4* | Resources | III-229 |
| $AFA | DlgFont | 2 | Fonts | III-228 |

## Open items

- **SCC/VIA addresses overlap.** The book prints `SCCRd` at $1D8, `VIA` at $1DA
  and `SCCWr` at $1DC, each holding a base address. Two 4-byte addresses cannot
  sit 2 bytes apart without overlapping, so one of these is a misprint or a
  different size. Resolve by owner observation or another published source;
  do not guess.
- `SysParam` ($1F8, 20 bytes) is the low-memory copy of parameter RAM; the `SP…`
  entries from $1F8 to $20B are fields inside it.
- `ABusVars`, `SPATalkA` and `SPATalkB` belong to AppleTalk, which postdates
  System 1. Other entries may also postdate January 1984; the book describes the
  May 1984 System file.
- Apple warns that this layout may differ in later system software (II-19) and
  on other models (III-17). A `system1-1984` contract will still need fixed
  addresses for applications that read them directly.
- The table lists only documented globals. Undocumented locations are out of
  scope unless owner observation records them.
