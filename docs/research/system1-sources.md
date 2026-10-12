# System 1 source foundation — 2026-10-11

Research record for TD-003 and R-001/R-004/R-009, prepared by Claude under the
owner's direction. It records which documents may inform System 1 contracts. It
is not a specification and accepts no historical behavior.

## Outcome

- **Eligible now:** Apple's *Inside Macintosh* Promotional Edition of March 1985
  (SRC-0068), the owner's copy of the 1985 Addison-Wesley *Inside Macintosh*
  Volumes I–III (SRC-0071) and Motorola's *M68000 Family Programmer's Reference
  Manual* (SRC-0069). All are published.
- **Excluded:** Apple's 1984 *Inside Macintosh* binders, the February 1984
  assembly-language draft and the February 1985 hardware draft (SRC-0070). Each
  says its distribution "does not constitute publication", and most pages are
  marked "CONFIDENTIAL". The owner chose published editions only (SRC-0067).
- **Applicability:** no eligible document is dated before System 1 shipped
  except one Promotional Edition chapter (QuickDraw, 3/2/83). The 1985 volumes
  describe ROM version 105 in the Macintosh 128K and 512K and the System file
  created May 2, 1984 (I-4). Every fact from them is a later-manual fact until
  owner-held reference observation confirms it for January 1984.
- **Low-memory map:** a [draft](system1-low-memory.md) from Volume III's
  Appendix D lists 138 documented globals by address. Two items stay open:
  Volumes I–III do not state the trap dispatch table's address, and the printed
  `SCCRd`/`VIA`/`SCCWr` locations overlap.
- **128K hardware:** Volume III chapter 2 (III-15–46) is the published hardware
  source for R-002/R-003 and the 128K feasibility study.

## Why the drafts are excluded

[POLICY](../clean-room/POLICY.md) admits historically published documentation
and assesses eligibility per source. The drafts' own copyright pages state that
limited distribution "does not constitute publication"; binder and assembly-draft
pages carry a "CONFIDENTIAL" footer; several chapters say "not for distribution".
That they are now scanned publicly does not make them published. A March 1985
Apple letter in the Promotional Edition answers a reader who bought the binder
version by mail order; the owner weighed that and still chose published editions. The exact
excluded files and the exposure that already occurred are listed in SRC-0070.

## R97-Q-01: dated manuals in the Promotional Edition

The Promotional Edition's own index lists each manual it contains with a date.
The table reproduces those dates and checks them against each chapter's
modification history and running page footers in the OCR text (SRC-0068). Dates
are the edition of the chapter in this book, not the date of the behavior.

| Code | Manual | Index date | Cross-check |
| --- | --- | --- | --- |
| RD | Inside Macintosh: A Road Map | 9/10/84 | History and footer agree |
| UI | Macintosh User Interface Guidelines | 11/30/84 | History and footer agree |
| MI | Macintosh Memory Management: An Introduction | 8/20/84 | History and footer agree |
| AL | Programming Macintosh Applications in Assembly Language | 1/22/85 | Third draft; first draft 2/27/84 |
| RM | The Resource Manager | 11/28/84 | History and footer agree |
| QD | QuickDraw | 3/2/83 | "Revised for ROM 2.1"; only pre-1984 manual |
| FM | The Font Manager | 6/11/84 | History and footer agree |
| EM | The Toolbox Event Manager | 11/19/84 | History and footer agree |
| WM | The Window Manager | 5/30/84 | History and footer agree |
| CM | The Control Manager | 5/30/84 | History and footer agree |
| MN | The Menu Manager | 9/24/84 | One page footer still reads 11/1/83 |
| TE | TextEdit | 1/14/85 | History and footer agree |
| DL | The Dialog Manager | 7/6/84 | History and footer agree |
| DS | The Desk Manager | 8/22/84 | History and footer agree |
| SM | The Scrap Manager | 1/31/85 | History and footer agree |
| TU | The Toolbox Utilities | 11/13/84 | History and footer agree |
| PK | Macintosh Packages | 5/7/84 | History and footer agree |
| MM | The Memory Manager | 10/9/84 | History and footer agree |
| SL | The Segment Loader | 8/24/84 | History and footer agree |
| OE | The Operating System Event Manager | 11/19/84 | History and footer agree |
| FL | The File Manager | 5/21/84 | History and footer agree |
| PR | Printing from Macintosh Applications | 6/15/84 | History and footers say 6/11/84 |
| DV | The Device Manager | 6/15/84 | History and footer agree |
| DD | The Disk Driver | 9/18/84 | History and footer agree |
| SN | The Sound Driver | 11/15/84 | History and footer agree |
| SR | The Serial Drivers | 9/28/84 | History and footer agree |
| AM | The AppleTalk Manager | 1/31/85 | History and footer agree |
| VR | The Vertical Retrace Manager | 6/15/84 | History and footer agree |
| EH | The System Error Handler | 9/26/84 | Footer agrees; history not legible in OCR |
| OU | The Operating System Utilities | 1/11/85 | History and footer agree |
| ST | The Structure of a Macintosh Application | 2/8/84 | History and footer agree |
| PT | Putting Together a Macintosh Application | 7/10/84 | Indexed; not in the book's 33-part contents list |

The book also includes Apple's *Apple Numerics Manual* (SANE), which it says
will be available separately, and an index updated through 2/1/85. OCR text is
imperfect; confirm any date that matters against the page image.

Conclusion for R-001/TD-003: the Promotional Edition is the earliest eligible
Apple programming manual found, and it is a later manual. It may supply
interface names, data-structure layouts and documented rules for specifications
marked "later manual, needs 1984 validation". QuickDraw is the one chapter dated
before the January 1984 release.

## R97-Q-04: low-memory map status

The Promotional Edition gives symbolic names, sizes and meanings of global
variables (in each chapter's summary), not their addresses; its assembly chapter
tells programmers to use the names from Apple's equate files. Those are
Apple-supplied definition source files, so they are not inputs here.

The 1985 volumes (SRC-0071) supply the published address list. Appendix D
(III-227–231) gives each documented global's location, and II-19–20 and
III-17–21 give the RAM layout and buffer addresses. The
[draft map](system1-low-memory.md) re-sorts the appendix by address; names and
addresses were checked against the page images.

- **Answered, as a later manual:** documented globals and their locations, for
  the May 1984 System file and ROM version 105.
- **Open:** the trap dispatch table's address (I-87–88 describe a 1024-byte
  table of 512 one-word entries in RAM but give no address); the overlapping
  `SCCRd` $1D8, `VIA` $1DA and `SCCWr` $1DC entries; which entries existed in
  January 1984 (the AppleTalk entries belong to a later product).
- **Out of scope unless observed:** undocumented locations. Black-box
  observation on owner-held 64-KB-ROM hardware or software, under a reviewed
  probe plan, can confirm or extend the table (R-001/R-004).

No address enters a specification until a specification adopts it with its
1984 status stated.

## Effect on earlier research

The 2026-10-07 *ClasSICK 0.97 Research Dossier* (an owner project document, not
in this repository) took its trap-word, dispatch-table and low-memory facts from
the excluded assembly draft and its 128K hardware facts from the excluded
hardware draft. Those statements are not eligible inputs. Each topic now has an
eligible replacement in SRC-0071; a fact is re-sourced only by reading it there,
not by keeping the dossier's wording.

| Dossier topic | Eligible locator (SRC-0071) |
| --- | --- |
| Pascal data type sizes for assembly language | I-86 |
| Trap dispatch table: size, entry encoding, RAM and ROM base addresses | I-87–88 |
| Trap mechanism, trap word format, trap macros | I-88–90 |
| Calling conventions: stack-based, register-based, result codes, saved registers | I-90–94 |
| RAM organization of the 128K and 512K | II-19–20 (Figure 9) |
| Documented global-variable locations | III-227–231 (Appendix D) |
| Address space overview: RAM, ROM at $400000, I/O space | III-17–18 |
| Video interface and screen buffers | III-18–20 |
| Sound generator and sound buffers | III-20–22 |
| SCC | III-22–25 |
| Mouse | III-25–29 |
| Keyboard and keypad, including their protocols | III-29–33 |
| Disk interface | III-33–36 |
| Real-time clock | III-36–39 |
| VIA | III-39–42 |
| System startup | III-42 |
| Hardware summary: constants, variables, exception vectors | III-43–46 |

The trap and dispatch topics also appear in the Promotional Edition's assembly
chapter (1/22/85).

## Next steps

- Claude: the trap-gateway draft is
  [SPEC-0017](../specifications/SPEC-0017-trap-gateway.md) (draft 0), and the
  [128K feasibility study](mac128k-feasibility.md) is done. Next, write a
  black-box probe plan for the dispatch-table address, the SCC/VIA locations,
  SPEC-0017's open questions and 1984 confirmation; hash and locate the
  Motorola manual sections when a local copy is approved.
- Owner: nothing is needed for these documents. Probes on real 64-KB-ROM
  hardware or software wait for a reviewed plan and the owner's go-ahead.
