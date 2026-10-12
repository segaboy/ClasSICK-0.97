# Asset replacement research — 2026-10-12

Research record prepared by Claude at the owner's request. It is not a
specification and changes no code. It lists every part of System 0.97 (the January
1984 Macintosh system software, sold as System 1.0) whose assets ClasSICK must
replace, rename or leave out to avoid legal entanglement, and what must stay the
same so that 1984 programs still work.

**This is research, not legal advice.** Claude is not a lawyer. Before any public
release, especially a commercial or non-US one, have an intellectual-property
lawyer review the "Ask counsel" items at the end.

How it was built: from published documentation and secondary histories only. No
Apple file, disk image or ROM was opened. Page numbers like "I-219" are printed
pages of the owner's 1985 *Inside Macintosh* Volumes I–III (SRC-0071), a
later manual describing the May 1984 System file, so the exact January 1984
inventory still needs owner observation.

## The short answer

1. **Everything Apple made must be replaced with original work.** That means all
   code (ROM, System file, Finder, desk accessories), all icons, cursors, fonts'
   letter shapes, decorative patterns, alert and startup artwork, sounds, sample
   content and any text longer than a short label.
2. **Apple's brand names and logo must stay out of ClasSICK's own name, screens
   and art.** Apple lists Apple, the Apple logo, Mac, Macintosh, Finder, Chicago,
   Geneva, New York, Monaco, QuickDraw, ImageWriter and LaserWriter among its
   trademarks. A plain-words statement such as "runs software written for the
   Macintosh 128K" is allowed under Apple's own guidelines, with conditions.
3. **What programs depend on should stay the same.** Trap names and numbers,
   resource types and ID numbers, font numbers, character codes, file and
   driver identifiers, data formats, text metrics and screen geometry are the
   interface. Courts have been more tolerant of reimplementing interfaces than
   of copying artwork or code, though no ruling covers every case.

The five places most likely to cause trouble, in order:

1. The apple symbol at character code $14, which every application uses as the
   title of its first menu.
2. The icons: trash can, folder, disk, document, alert icons and the startup
   pictures.
3. The four trademarked font names and the bitmap letter shapes.
4. Apple names inside on-screen text, such as "Welcome to Macintosh." and
   "About the Finder".
5. The desktop as a whole, if it ends up a near-copy of the 1984 screens.

## Legal background in brief

| Principle | Meaning for ClasSICK | Source |
| --- | --- | --- |
| Program code is protected, including object code in ROM and operating systems | Every byte of code must be independently written. The clean-room BIOS makers of 1984 are the model | *Apple v. Franklin*, 714 F.2d 1240 (3d Cir. 1983) |
| Ideas, procedures, systems and methods of operation are not protected; only their expression is | Windows, menus, icons as a concept, and opening and closing things are free to use; specific artwork is not | 17 U.S.C. §102(b) via Copyright Office Circular 33; *Apple v. Microsoft*, 35 F.3d 1435 (9th Cir. 1994) |
| A user interface's overall "look and feel" got only thin protection; infringement needed near-identity of the works as a whole | Matching the 1984 behavior is safer than matching the 1984 pictures | *Apple v. Microsoft* (9th Cir. 1994) |
| Specific icons can infringe | At trial, the trash can and folder icons in HP's NewWave were found to infringe Apple's | *Apple v. Microsoft* (district court), per Wikipedia |
| A menu command hierarchy was held to be an unprotected method of operation | Command names and their order can probably be kept for compatibility; the ruling binds one circuit only, and the Supreme Court split 4–4 without setting precedent | *Lotus v. Borland*, 49 F.3d 807 (1st Cir. 1995), aff'd by an equally divided Court, 516 U.S. 233 (1996) |
| Reimplementing an interface's declarations for programmers was fair use | Supports keeping trap names, routine names and parameter lists; the Court did not decide whether interfaces are copyrightable | *Google v. Oracle*, 593 U.S. 1 (2021) |
| Words and short phrases, names, typeface designs, and familiar symbols or simple patterns are not copyrightable | Menu words, button labels, a checkerboard gray and the shapes of letters as a design are free in the US | Copyright Office Circular 33 |
| Font *software* can be protected; bitmap font data has an unsettled status | Never copy Apple's font data; draw original letters | Copyright Office 1992 rule on typeface programs; *Adobe v. Southern Software* (1998), per Monotype's summary |
| Apple restricts its logo, lookalike apples and its marks in product names; referential "compatible with" use is allowed with conditions | No apple-shaped symbol anywhere; no Apple marks in names; compatibility statements in plain words, less prominent than our name, no implied endorsement | Apple's trademark list and third-party guidelines; nominative fair use, *New Kids on the Block v. News America*, 971 F.2d 302 (9th Cir. 1992) |
| Works made for hire are protected for 95 years from publication | Apple's 1984 artwork and code stay protected until about 2079. Susan Kare designed the icons and fonts as an Apple employee (1983–86), so they belong to Apple | 17 U.S.C. §302(c); Wikipedia, "Susan Kare" |
| Patents from the mid-1980s have expired | Patents do not block reimplementation now. Reading a patent as a source is still a separate project decision; the region patent has not been opened | Patent terms under 35 U.S.C. §154 |
| History | In 1985 Digital Research settled with Apple by removing GEM's trash-can icon, disk icons and the close box in the window's upper-left corner. That was a settlement, not a ruling, but it shows what Apple pursued | The Digital Antiquarian, "Doing Windows, Part 6" |

Other countries treat typeface designs, user interfaces and trademarks
differently. Check before any release outside the United States.

## Component by component

Each table says what the 1984 asset was, what ClasSICK must do, and what has to
stay the same for programs.

### 1. ROM (the 64 KB of built-in software)

| 1984 asset | Action | Keep for compatibility |
| --- | --- | --- |
| Toolbox, Operating System and QuickDraw code (I-87) | Independent implementation (already project policy) | Trap words, trap numbers and register/stack conventions (SPEC-0017) |
| Question-mark disk and "sad Macintosh" startup pictures (III-42, II-362) | Original pictures | When each appears |
| Any startup sound | Our own generated tone | — |
| System font in memory (`ROMFont0`, I-233; named Chicago, I-219) | Original font with an original name (section 5) | Font number 0, 12-point system size, metrics (section 5) |
| Standard arrow cursor (QuickDraw global `arrow`, I-163) | Original 16 × 16 drawing | Size, mask, hot-spot meaning |
| Standard patterns white, black, gray, ltGray, dkGray (QuickDraw globals, I-145) | Keep: they are plain fills and checkerboards, which are familiar designs, and programs use them by name | Their bit values |
| Disk driver name `.Sony`, reference −5 (II-213); printer driver `.Print` (II-162) | Keep only as internal identifiers that programs open by name; never as visible branding ("Sony" is Sony's mark) | The exact names and numbers. Ask counsel |

### 2. System file (resources loaded from the startup disk)

| 1984 asset | Action | Keep for compatibility |
| --- | --- | --- |
| Fonts, type `FONT` (I-218) | Original fonts and names (section 5) | Font numbers, sizes offered, metrics |
| Desk accessories, type `DRVR` (I-107, I-443) | Original programs (section 4) | Driver interface and messages (I-448) |
| Packages, type `PACK`: Disk Initialization, Standard File, floating-point, transcendental functions, International Utilities, binary–decimal conversion (I-483) | Original code. The floating-point package follows the public IEEE 754 standard (I-483) | Package numbers, selectors, results |
| Standard File dialogs (template ID −3999 and others, I-521) | Original layout and look | Dialog item numbers and types, since applications may supply their own version of the template |
| Disk Initialization dialogs | Original look and wording | Behavior and results |
| Cursors I-beam, cross, plus and watch, type `CURS` IDs 1–4 (I-474) | Original drawings. The watch is distinctive artwork and needs a different design | IDs, 16 × 16 size, mask, hot spot |
| 38 standard patterns, `PAT#` ID 0 (I-473) | Original set; plain grays and checkerboards may match | ID, count of 38, 8 × 8 size |
| Desktop pattern (resource `deskPatID`, I-282) | Original or a plain gray | Resource ID and use |
| Alert icons stop, note and caution, `ICON` IDs 0–2 (I-420, I-432) | Original icons | IDs, 32 × 32 size |
| System error alerts: startup table `DSAT` 0 and user table ID 2 (II-359), with the "Welcome to Macintosh." picture and message and the bomb with "Sorry, a system error occurred." (II-357–358) | Original pictures; original wording with no Apple names | Table resource IDs, error ID numbers and their meanings (II-364) |
| Window, control and menu definition procedures (`WDEF`, `CDEF`, `MDEF`, I-107) | Original drawing | Part codes, regions and sizes programs compute with (section 7) |
| International resources `INTL` 0 and 1 (I-495) | Keep the values (date, time and number formats are facts) | Formats and IDs |
| RAM serial driver `SERD` (I-107) | Original code | Interface |
| Keyboard-to-character mapping | Keep the mapping (functional facts) | Key codes (III-31–32) and character codes (I-220) |

### 3. Finder (the desktop program)

| 1984 asset | Action | Keep for compatibility |
| --- | --- | --- |
| The name "Finder" (Apple registered trademark) | Give ClasSICK's desktop program an original name. Applications quit back to whatever program name is stored in `FinderName` (II-59), so ours works there | The `FinderName` mechanism |
| Program code | Independent implementation | Documented file information and desktop behaviors |
| Icons: disk, folder, trash (empty and full), generic document, generic application, System file and others | Original icons. The trash and folder are the riskiest: courts and the GEM settlement both singled them out | Icon size (32 × 32 with mask) and the meaning of each icon state |
| Menus: Apple, File, Edit, View, Special (Wikipedia) | Keep the plain words and the Edit commands. Name the Apple menu by our emblem, not an apple (section 5) | Edit command order Undo, Cut, Copy, Paste, Clear (`SystemEdit` command numbers, I-441) |
| "About the Finder" box and credits | Original about box naming our program | — |
| Overall desktop appearance | Original decoration (section 7) | Geometry |
| "Empty Folder" | Generic words; keep or rename | — |

### 4. Desk accessories (Apple menu mini-programs)

All seven need original code and artwork. Their names are ordinary words, not on
Apple's trademark list; keeping them is low risk, and renaming is optional.

| 1984 accessory | Replace | Notes |
| --- | --- | --- |
| Alarm Clock | Clock face, menu-bar flash | The "flash the apple" behavior (II-198) flashes our emblem instead |
| Calculator | Key layout drawing | A four-function keypad is a functional layout; draw it ourselves |
| Control Panel | All symbols (it used pictures instead of words) | Settings and their parameter-RAM fields stay |
| Key Caps | Keyboard drawing | The key layout itself is a hardware fact |
| Note Pad | Page artwork | Keep the file format if programs read it |
| Puzzle | Tile art | The 15-tile sliding puzzle is a public-domain game |
| Scrapbook | Artwork and any sample content in the shipped Scrapbook file | Ship our own sample content or none |

### 5. Fonts and special characters

| 1984 asset | Action | Keep for compatibility |
| --- | --- | --- |
| Font names Chicago, Geneva, New York, Monaco (Apple registered trademarks) | Original names | Font numbers 0 (system), 1 (application, Geneva by default), 2, 3 and 4 (I-219) |
| Other documented names: Venice, London, Athens, San Francisco, Toronto, Cairo, Los Angeles (I-219) | Original names if ClasSICK offers fonts in these slots | Their numbers |
| Letter shapes of every font | Draw original glyphs. US law does not protect typeface designs, but Apple's bitmap data may still be protected, and its fonts are Kare's distinctive designs | — |
| Metrics: character widths, ascent, descent, leading, system font size 12 | Match them where programs depend on them: dialog and menu layouts were sized for the system font. Metric work needs an approved observation protocol first | The numbers themselves. Ask counsel whether to match width tables exactly |
| Special characters $11 command symbol, $12 check mark, $13 diamond, $14 apple (I-220) | Draw our own. $14 must not be an apple or anything resembling one: Apple's guidelines bar "an image of a real apple or other variation of the Apple logo" | The codes. Applications set their first menu's title to character $14 (I-341, I-351), so our system font must draw a ClasSICK emblem there |
| Which fonts and sizes were on the System 0.97 disk | Not established from eligible sources | Owner observation of a running system's Font menu, not opening Apple's files |

The command symbol is an old public-domain sign (Unicode "place of interest
sign"), so an original drawing of it is fine.

### 6. Startup disk files

The System 1.0 disk held six files: System, Finder, Clipboard, an ImageWriter
printer driver, Scrapbook and Note Pad (Wikipedia, "System 1").

| File | Action | Keep for compatibility |
| --- | --- | --- |
| System | Our own content; the generic name can stay | Name in `SysResName` (I-134) |
| Finder | Renamed desktop program (section 3) | `FinderName` holds our name |
| Clipboard File | Generic name; keep | Name used by the Scrap Manager (`ScrapName`, I-457) |
| Imagewriter | Our own printer driver, if any, named without the ImageWriter mark. Saying "prints to ImageWriter printers" is referential use | `.Print` driver interface (II-162) |
| Scrapbook File, Note Pad File | Our own contents (empty or original samples) | File formats |

### 7. Screen appearance

Keep the geometry programs compute with: the 20-pixel menu bar with its 1-pixel
border (I-341), window and control part codes and the regions they report,
standard control and scroll-bar sizes, icon and cursor sizes, and the 512 × 342
screen. Change the decoration: title-bar striping, close-box and grow-box
drawings, scroll arrows and thumbs, button corners, desktop pattern and the
rounded screen corners if desired. Record each intentional difference, as
`docs/architecture/system1.md` already requires.

Keeping the close box in the upper-left corner and the trash on the desktop are
ideas rather than artwork under *Apple v. Microsoft*, but GEM gave up exactly
those in 1985. Keep them for usability only after counsel agrees.

### 8. Text on screen

- **Keep:** menu titles and commands, button labels (OK, Cancel, Open, Eject,
  Drive, Save) and other short functional labels. Short phrases are not
  copyrightable, and programs and users rely on them.
- **Rewrite:** every system message that names Apple or Macintosh ("Welcome to
  Macintosh."), the system error messages ("Sorry, a system error occurred.",
  "Can't load the finder", II-357), about boxes and any help text. Even where a
  short message is not protected, our own wording avoids implying it is Apple's.
- **Never** put Apple, Macintosh, Mac, Finder or QuickDraw in ClasSICK's own
  name, window titles, about boxes or icons. Documentation may say "implements
  the routines documented in *Inside Macintosh*" as a plain reference.

### 9. Sounds

The 1984 system made simple generated beeps (system beep, alarm). Generate our
own; never include Apple audio. Simple tones carry little risk.

### 10. Documentation, screenshots and samples

- Cite *Inside Macintosh* by page; quote only briefly; never republish it (project
  rule).
- No screenshots of real System 1 in public repositories (POLICY); public test
  images are independently drawn.
- No Apple sample code, sample documents or Guided Tour material.

### 11. The project's own name

"ClasSICK" is not on Apple's trademark list, but Apple has used "Classic" for
products (the Macintosh Classic and Mac OS X's Classic environment). Low risk as
written; ask counsel before any commercial use, and never pair it with an Apple
mark.

## What must stay the same (the compatibility interface)

- Trap words, numbers, names and calling conventions (SPEC-0017; I-85–95).
- Resource types and the documented IDs above; package numbers (I-483).
- Font numbers (I-219) and character codes, including $11–$14 (I-220).
- File and driver identifiers: `System`, `Clipboard File`, `.Sony`, `.Print`,
  and the `FinderName` mechanism.
- Low-memory global addresses (the draft low-memory map).
- Edit menu command numbers (I-441); system error IDs (II-364).
- Text metrics where layouts depend on them, and screen and window geometry.
- Pattern globals' values (I-145) and cursor and icon sizes and masks.

## Ask counsel

1. Matching Apple's font metrics exactly (width tables) versus approximating
   them.
2. Keeping the 1984 menu words, Edit menu order and Finder menu structure
   (*Lotus* is not binding outside the First Circuit).
3. Keeping the close-box position and a trash can on the desktop.
4. Using `.Sony`, `.Print`, `System` and `Clipboard File` as internal identifiers.
5. Whether historical font names may be accepted as lookup aliases (for programs
   that ask for "Geneva" by name) while never being displayed.
6. The project name and any compatibility wording on packaging or web pages.
7. Non-US release (typeface, design and interface law differ).

## Recommended next steps

1. Add an asset register to the public repository (for example
   `docs/compatibility/assets.md`): one row per asset with purpose, the
   compatibility constraints above, author, license, source files and review
   status. The repository guard cannot detect copied pixels, so each asset needs
   human review.
2. Choose original names: the desktop program, the system and application fonts,
   the monospaced font, and optionally the desk accessories.
3. Design the ClasSICK emblem for character $14 before any Apple-menu work.
4. Draw in this order: system font, arrow and the four standard cursors, alert
   icons, startup pictures, desktop icons, patterns.
5. Record a black-box observation protocol for font metrics and the exact System
   0.97 inventory before relying on either.

## Sources

- Owner's *Inside Macintosh* Volumes I–III (Addison-Wesley, 1985), SRC-0071;
  page numbers as cited.
- [Apple trademark list](https://www.apple.com/legal/intellectual-property/trademark/appletmlist.html)
- [Apple guidelines for using Apple trademarks and copyrights](https://www.apple.com/legal/intellectual-property/guidelinesfor3rdparties.html)
- [Copyright Office Circular 33, Works Not Protected by Copyright](https://www.copyright.gov/circs/circ33.pdf)
- [Monotype, font software copyrightability in the United States](https://www.monotype.com/resources/expertise/case-fonts-copyrightability-font-software-united-states)
- [Apple Computer, Inc. v. Franklin Computer Corp. (Wikipedia)](https://en.wikipedia.org/wiki/Apple_Computer,_Inc._v._Franklin_Computer_Corp.)
- [Apple Computer, Inc. v. Microsoft Corp. (Wikipedia)](https://en.wikipedia.org/wiki/Apple_Computer,_Inc._v._Microsoft_Corp.)
- [Lotus Development Corp. v. Borland International (Wikipedia)](https://en.wikipedia.org/wiki/Lotus_Development_Corp._v._Borland_International,_Inc.)
- [Google LLC v. Oracle America, Inc. (Wikipedia)](https://en.wikipedia.org/wiki/Google_LLC_v._Oracle_America,_Inc.)
- [Nominative use (Wikipedia)](https://en.wikipedia.org/wiki/Nominative_use)
- [The Digital Antiquarian, "Doing Windows, Part 6: Look and Feel"](https://www.filfre.net/2018/07/doing-windows-part-6-look-and-feel)
- [System 1 (Wikipedia)](https://en.wikipedia.org/wiki/System_1)
- [Fonts on Macintosh (Wikipedia)](https://en.wikipedia.org/wiki/Fonts_on_Macintosh)
- [Susan Kare (Wikipedia)](https://en.wikipedia.org/wiki/Susan_Kare)
- 17 U.S.C. §102(b), §302(c); 35 U.S.C. §154 (statutes cited from general
  knowledge; the official text was not retrieved in this session).
