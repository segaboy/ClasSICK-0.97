# Gemini January 1984 baseline report: audit — 2026-10-13

Research record prepared by Claude at the owner's request. It is not a
specification and assigns no stable identifiers; the implementation lead
decides whether to register the report as a source and what, if anything, to
adopt. It changes no code.

The owner commissioned Gemini to research the January 1984 version identity and
user-visible experience (open question R-001 in [questions](questions.md)) and
supplied its report for audit. This record publishes the audit, not the report:
the report's claims, labels and proposed resolutions are research content, and
none becomes a ClasSICK fact by appearing here.

## Intake

| Supplied file | Bytes | SHA-256 |
| --- | --- | --- |
| classick_jan1984_baseline_research.md | 34,883 | `97974f8d8ec2763621072c108fa60a4043608203d4a56f8b4b885d98eb72d234` |

The original is kept unchanged outside Git.

The report has eight sections: a ten-entry source register (G-1 to G-10),
version identity, System Disk contents, a user-visible behavior inventory, a
1984–1985 change history, source conflicts, open questions and a clean-room
attestation.

Gemini attests that it used no Apple code, disk images, emulator source or the
excluded 1984–1985 drafts (SRC-0070). That attestation cannot be independently
established. No prohibited input is identified in the text, and its *Inside
Macintosh* locators use the bound edition's volume-page scheme, not the drafts'.

## How this audit was done

| Source | Identity verified 2026-10-13 | Pages read for this audit |
| --- | --- | --- |
| **SRC-0071**, *Inside Macintosh* I–III (1985), owner's PDF | SHA-256 `b65fe8ef…1d97`; 1,284 pages; I = PDF − 10, II = PDF − 568, III = PDF − 1003 | I-4, I-55, I-58, I-219, I-247, I-353, I-453–454, I-457; II-79, II-81, II-85, II-192, II-357–358, II-362, II-364–365, II-369–374, II-385; III-10, III-18–19, III-42, III-259. Page images checked: I-55, I-58, II-358. |
| **SRC-0068**, Promotional Edition (March 1985), local scan | SHA-256 `92490207…b237`; 1,326 pages; each chapter paged separately | UI-42 (11/30/84), EH-4 and EH-13 (9/26/84), all checked as page images. |

Web checks, 2026-10-13:

- Both links Gemini gives, for G-1 (the owner's guide) and G-3 (*Byte*), refused
  automated reading. They were not read, and no mirror or copy was used instead.
- One secondary article was read: Andrew Conachey, "What Software Came Bundled
  with the Earliest Macs?", [Low End Mac](https://lowendmac.com/2006/what-software-came-bundled-with-earliest-macs/),
  2006-01-03.
- A forum thread returned an access-denied page.
- The search results also included a site that distributes Apple disk images
  and a screenshot archive. Neither was opened.

Outcome labels:

- **VERIFIED**: an eligible 1985 manual says so. That is a later-manual fact,
  not evidence of January 1984 behavior.
- **CONFLICT**: an eligible manual says otherwise.
- **UNVERIFIED**: rests only on sources this audit could not read.
- **REJECTED**: not usable, whatever its source.

Edition tags follow [first-application interfaces](first-application-interfaces.md):
**[BOTH]**, **[DOC85]**, **[DIFF]** and **[INF]** (inferred from documented facts).

## In short

1. **R-001 stays open.** Nothing in the report or this audit establishes "System
   0.97" from an eligible primary source.
   - Gemini's main support is G-8, a 2001 Apple retrospective. Gemini gives no URL
     or excerpt for it, yet labels it primary and rates the conclusion "High".
   - The Low End Mac article (2006) attaches ".97" and the date 14-Jan-84 to the
     System file, without a source. Gemini attaches the same date to the Finder's
     About box. Secondary sources also disagree on when System 1.1 shipped
     (finding J24).
2. **Most of Gemini's *Inside Macintosh* locators are wrong.** Of eight citations
   checked, two are right (II-357; II-81 for file names), five are wrong and one
   is only partly right. Corrected locators are in the table below.
3. **Seven claims conflict with the 1985 manuals**:
   - the system error alert's buttons;
   - mouse scaling;
   - how the Clipboard survives an application launch;
   - Undo after Cut or Copy;
   - a key equivalent for Select All;
   - the Apple menu order;
   - the May 1984 System file date.
4. **Some later-manual facts confirm parts of the report**:
   - the seven desk accessories;
   - Chicago 12 as the system font and Geneva as the default application font;
   - speaker volume 0–7, with the menu bar blinking at 0;
   - the system error message and its ID;
   - names of 1–27 characters for volumes and 1–255 for files (under 64 for the
     Finder);
   - the 512 × 342 screen;
   - about 86 KB of application space.
5. **Most of the report is unverified.** The Finder menus and command keys,
   desk-accessory behavior, Control Panel controls, System Disk contents,
   installed fonts and the change history rest on the owner's guide, *Byte*
   and online summaries. Neither Gemini (by its own register) nor this audit
   read those pages. The report is a good checklist of what to confirm, not
   evidence.
6. **Two items are rejected**: the "hardcoded `Str63`" explanation, which is
   speculation about Apple's implementation, and the classification of "0.97"
   as a published, high-confidence fact.

## Findings

| # | Report claim (section) | Outcome | Evidence and note |
| --- | --- | --- | --- |
| J1 | Source register (1) | Process defect | No source has a hash or an identified copy. G-2, G-4 and G-6 are "inspected online summary"; G-7 was "inspected online" with no copy named. G-8, a 2001 retrospective without a URL, is marked as a primary published source. G-9 is never cited. G-10, which the register itself marks secondary, supports claims labelled [PUB]. Every claim from a source other than *Inside Macintosh* therefore starts as UNVERIFIED. |
| J2 | ROM revision $69 (105), citing II-374 (2.1) | VERIFIED, locator wrong | I-4: "version 105 of the ROM in the Macintosh 128K or 512K". II-385 (Environs): the ROM version word is $00xx on those machines. Reading 105 as decimal gives $0069 **[INF]**; the manual does not state the base. II-374 covers result codes and HandToHand. [DOC85] |
| J3 | ROM chip part numbers (2.1) | UNVERIFIED | Not in the pages checked, and no specification needs them. |
| J4 | "Finder 1.0", About-box credits and "14 Jan 84" (2.1) | UNVERIFIED | Rests on G-1, G-5 and G-10. Low End Mac (secondary, unsourced) instead gives "Version .97 14-Jan-84" for the System file. |
| J5 | "0.97" is an internal tag that Apple codified, rated High [PUB] (2.2, 6.1) | REJECTED as stated | A 2001 retrospective is not contemporary evidence, and Gemini quotes none of it. Gemini also says the System file showed users no version, so a user-visible "0.97" is unsupported. Keep it as an unverified lead under R-001. |
| J6 | 1984 literature called it "System 1.0" (2.2) | UNVERIFIED | No page or quotation is given. |
| J7 | System file dates (2.1, 5) | CONFLICT | I-4: the 1985 manual describes "the System file whose creation date is May 2, 1984", the only dated System file in an eligible source. Gemini dates the May 1984 release's System file April 24, 1984, from summaries. Neither date describes January 1984. |
| J8 | Disk and cassette part numbers (3) | UNVERIFIED | |
| J9 | Files on the System Disk (3.1) | UNVERIFIED and incomplete | III-10 [DOC85]: the Finder keeps an invisible resource file named Desktop, which the list omits. I-457: the scrap file is "usually 'Clipboard File'" (Gemini cites II-123, an MFS directory page). The Empty Folder is not in either manual. |
| J10 | Seven desk accessories (3.2) | VERIFIED | II-192 Figure 3 (the unit table) gives Calculator, Alarm Clock, Key Caps, Puzzle, Note Pad, Scrapbook and Control Panel unit numbers 12–18. I-55 Figure 16 shows the same seven in an application's Apple menu. [DOC85] |
| J11 | Apple menu in alphabetical order (4.2) | CONFLICT (weak) | I-55 Figure 16 (an application's menu, 1985 guidelines) orders them Scrapbook, Alarm Clock, Note Pad, Calculator, Key Caps, Control Panel, Puzzle. AddResMenu (I-353) states no order. This shows only that the one published illustration differs, not what the Finder showed. |
| J12 | Installed fonts and sizes, citing I-247 (3.3) | Partly VERIFIED, locator wrong | I-247 is the Event Manager's character-code chart. I-219: the system font is Chicago, "fixed at 12 points"; the application font is Geneva by default (also II-370). I-219's font-number list includes fonts that shipped later, so it is not a 1984 inventory. All other sizes are UNVERIFIED, and the open item in [asset replacement](asset-replacement.md) stands. |
| J13 | Startup sequence and error states (4.1) | Partly VERIFIED | III-42 [DOC85]: the question-mark disk icon shows until a disk is inserted; if disk startup fails, the "sad Macintosh" icon shows and the machine loops until switched off. II-357–358 and EH-4 [BOTH]: "Welcome to Macintosh" is a startup alert box, not a titled window. **[DIFF]** EH-4 also describes a startup alert with Eject and Restart buttons when no startup disk is found; II-357 omits it. Both summaries list a "this startup disk is not usable" alert (II-365; EH-13). Gemini's eject-with-X sequence, happy Mac, chime and "eight-character" sad-Mac code are UNVERIFIED; neither edition mentions them. |
| J14 | Finder menus and command keys (4.2) | UNVERIFIED, one CONFLICT | Neither manual documents the Finder's menus. The standard application Edit menu [BOTH] (I-58 Figure 19; UI-42 Figure 19): Undo ⌘Z, Cut ⌘X, Copy ⌘C, Paste ⌘V, Clear, Select All and Show Clipboard, with **no key equivalent for Select All**. Gemini's ⌘A conflicts with that standard, though it does not prove what the Finder had. **[DIFF]** 1985 puts Select All above the divider and Show Clipboard below it; 11/30/84 puts Show Clipboard and then Select All, both below. |
| J15 | Put Back, Close All, Clean Up, Erase Disk, View menu, renaming (4.2, 4.3, 6.3–6.4) | UNVERIFIED | Gemini resolves its Clean Up and Put Back "conflicts" only with G-1 page numbers that nobody here could read. II-85 [DOC85] records a file's location as the Trash window (−3), the desktop (−2), the disk window (0) or a folder. That is consistent with items in the Trash and on the desktop; it is not evidence of any command. |
| J16 | Trash emptied when an application launches; Trash does not bulge (4.3) | UNVERIFIED | The 2026-10-07 dossier's discovery-grade Wikipedia reading said "at shutdown or when the Finder quit", which is compatible but not the same claim. |
| J17 | Desk-accessory behavior, including Note Pad's eight pages (4.4, 4.8) | UNVERIFIED | |
| J18 | Control Panel ranges, citing II-371–374 (4.5) | Partly VERIFIED, one CONFLICT | Parameter RAM is at II-369–372; II-373–374 cover queues and data types. VERIFIED: speaker volume runs from 0 (silent) to 7 (loud) (II-371), and at 0 SysBeep "causes the menu bar to blink once" (II-385). II-370–372 give stored formats and defaults, not Control Panel positions: auto-key threshold in four-tick units (default 24 ticks); auto-key rate in two-tick units (default 6 ticks); caret blink and double-click time in four-tick units (default 32 ticks each); menu blink 0–3 (default 3), which Gemini omits; mouse scaling on or off (default on). **CONFLICT**: with scaling on, the cursor moves twice as far when the mouse moves more than a threshold, normally six pixels, in a sixtieth of a second (II-372). Gemini says "exponentially more". UNVERIFIED: the Control Panel's own settings and positions, a 12/24-hour clock, the date format and the pattern editor. |
| J19 | Clipboard behavior, citing II-77–85 (4.6) | Two CONFLICTS, locator wrong | II-77–85 are File Manager pages. I-453–454 [DOC85]: the desk scrap normally stays in memory and goes to disk only when memory is short. It must be back in memory when the application ends, and the Segment Loader carries it through the next launch in memory. Gemini says launches preserve it through the Clipboard File. I-58 and UI-42 [BOTH]: "The previous contents are kept around in case the user chooses Undo". Gemini says they are permanently overwritten. VERIFIED: there is one Clipboard, and Show Clipboard toggles with Hide Clipboard (I-58). |
| J20 | System error alert (4.7) | VERIFIED, one CONFLICT | II-357–358, II-362 and EH-4 [BOTH]: "Sorry, a system error occurred", with the error ID at the bottom right. ID 2 is an address error (II-364; EH-13). **CONFLICT**: both editions say the alert usually has Restart **and** Resume buttons; Gemini gives a single Restart. If the Finder cannot be found, the alert reads "Can't load the finder" with only Restart (II-357; EH-4). |
| J21 | Other alert texts, including the Erase Disk confirmation quoted from G-1 p. 112 (4.7) | UNVERIFIED | Not in either manual. II-358 adds a File Manager "Please insert the disk:" alert, shown in a dialog box, which Gemini does not list. |
| J22 | Limits stated to users (4.8) | Mixed | VERIFIED: 512 × 342 screen and 21,888-byte buffer (III-19); 400K disk (II-79); volume names of 1–27 printing characters (II-79, III-259; Gemini cites II-81); file names of 1–255 printing characters without colons, kept under 64 for the Finder (II-81). Gemini's "about 86 KB" agrees with the 88,320 bytes (86.25 KB) of application space without alternate buffers in II-19–20 Figure 9 ([128K feasibility](mac128k-feasibility.md)). UNVERIFIED: 800 sectors (not checked) and "100 to 128 files" (no source). The 63/255 "conflict" in 6.2 is not one: II-81 states both. |
| J23 | Finder and TextEdit "hardcoded to `Str63`" (6.2) | REJECTED | This guesses at Apple's internal implementation, which [POLICY](../clean-room/POLICY.md) excludes. The [filename review](mfs-filenames-lead-review.md) (F1) already found that II-81 does not set an exact Finder 1.0 boundary. |
| J24 | May 1984 update: System 1.1/Finder 1.1g, Set Startup, swap and boot-time percentages, rename protection, Font Mover (5) | UNVERIFIED | G-6 was read only as a summary. Do not adopt the percentages without the insert's own text. The dates disagree: Gemini and Low End Mac give May 1984, while the 2026-10-07 dossier's Wikipedia reading gave 29 December 1984. I-4's May 2, 1984 System file is the only eligible dated anchor. |
| J25 | 512K (September 1984) and System 2.0/Finder 4.1 (April 1985) (5) | UNVERIFIED; outside 0.97 | Useful only as a list of candidate later features to keep out of the 0.97 profile once confirmed: New Folder, Shut Down, MiniFinder, Print Catalog and Set Startup. |
| J26 | Open questions: flux-image a sealed disk for the System file's timestamp; transcribe the Guided Tour cassette (7) | Method not adopted | Imaging a System Disk would copy Apple executables into a workspace, and transcribing the tape would reproduce a recording. The existing method (below) reaches the same answers through what the Finder displays. |

## What to carry forward

1. **Checklist for confirmation.** Sections 3 and 4 of the report make a usable
   checklist. Confirm each item from a primary source or owner observation, and
   record which:
   - the Finder's five menus, their commands and key equivalents;
   - About-box text;
   - System Disk files, including the Empty Folder;
   - installed fonts and sizes (the Font menu of a running application);
   - startup screens and alerts;
   - Trash behavior;
   - each desk accessory's behavior;
   - Control Panel controls and their positions;
   - disk-copy swapping.
2. **Primary sources to obtain.** These are the sources the report leans on.
   Attaching any of them lets it be hashed, page-mapped and checked the way
   SRC-0071 was:
   - the 1984 *Macintosh* owner's guide (exact part number and edition);
   - *Byte*, February 1984 (Williams);
   - *Macworld*, February 1984;
   - the January 1984 System update insert;
   - the May 1984 "Software Enhancements" insert.
   The owner's guide and *Byte* hosts that Gemini lists refuse automated
   reading.
3. **R-001 method, unchanged from earlier research.**
   - On legitimately held original 1984 media, the owner records only what the
     Finder displays: the About box and Get Info (name, version, created and
     modified dates), plus the media's identity.
   - Do not image the disk or inspect resources.
   - Because this would be a reference-system session, it needs a reviewed probe
     plan and the owner's go-ahead.
   - Gemini's two version stories, the Low End Mac claim and the April 24 / May 2
     dates can all be settled this way.
4. **Later-feature exclusions** (J25) become profile rules only after confirmation.

## Boundaries

- No Apple ROM, System, Finder, disk image, screenshot or sample code was used.
- No emulator or VM was run.
- No excluded draft was opened.
- Hosts that refused automated reading were not worked around.
- *Inside Macintosh* figures were read for their facts. No artwork, icon or glyph
  was copied.
- Quotations are a few words each.
- Interfaces, specifications, R-001 and the asset register are unchanged.
- Lead review of this audit is pending.
