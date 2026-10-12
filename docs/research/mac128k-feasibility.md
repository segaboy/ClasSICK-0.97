# Macintosh 128K feasibility study — 2026-10-11

Research record for TD-001, R-002 and R-003 (milestone M0.3), prepared by Claude.
It is not a specification and changes no code. Apple facts come from the owner's
1985 *Inside Macintosh* Volumes I–III (SRC-0071), a later manual that describes
the Macintosh 128K and 512K, ROM version 105 and the May 1984 System file; page
numbers are its printed pages. One third-party claim comes from SRC-0072. Code
sizes are this project's own measurements.

## Answer in brief

- **RAM is tight but countable.** Apple's documented layout leaves 88,320 bytes
  (86.25 KB) of application space on a 128K. ClasSICK must keep that layout, so
  everything of ours that lives in RAM has to fit where Apple's system already
  lived: the 2,816 bytes of system globals and the system heap, typically
  17,152 bytes.
- **ROM decides it.** ClasSICK's Toolbox and Operating System code has to fit
  in the replacement ROM, as Apple's did, or it takes memory away from
  applications. Apple's entire ROM fits within 64 KB (I-88). A replacement-ROM
  vendor states that
  128K and 512K boards run 128 KB of ROM code (SRC-0072). If that holds on a
  stock 128K board, ClasSICK gets twice Apple's code budget.
- **Startup is documented well enough to begin.** Volume III covers the reset
  overlay, video, sound, VIA, keyboard, mouse, clock and interrupt vectors. The
  floppy's on-disk encoding is not documented, so the first 128K boot must not
  need a floppy.
- **Verdict:** not yet proven. Plausible with 128 KB of ROM; unlikely with
  64 KB, because ClasSICK is written in C and Apple's ROM was hand-written
  assembly (an assumption to be tested by measurement, not a measured fact).

## Published facts used

| Fact | Locator |
| --- | --- |
| MC68000 at 7.8336 MHz; 16 MB address space in four 4 MB blocks: RAM, ROM, SCC, IWM/VIA; devices repeat within their block | III-17 |
| RAM at 0; the first 256 bytes are exception vectors | III-17 |
| Video, sound and disk-speed hardware share RAM; CPU access to RAM averages about 6 MHz, ROM runs at the full 7.83 MHz | III-17–18 |
| ROM base $400000 (`ROMBase`); the I/O region starts at $800000 | III-18 |
| The whole ROM lies within 64 KB of `ROMBase` | I-88 |
| Screen 512 × 342, 1 bit per pixel, 21,888 bytes, bit 15 of each word leftmost; main buffer $1A700, alternate $12700 | III-19 |
| 60.15 Hz frames: 704 pixel times per line at 15.6672 MHz, 370 lines including 28 of vertical blanking | III-18–19 |
| Sound: 370 words per frame, high bytes for sound, low bytes drive disk motor speed; main $1FD00, alternate $1A100 | III-20–21 |
| RAM layout of the 128K: 2,816 bytes of system globals, system heap, application space, alternate and main buffers, 128 bytes for the System Error Handler | II-19–20, Figure 9 |
| System heap typically 16.75 KB on a 128K; its size comes from the startup disk's system startup information | II-9, II-120 |
| Reset: the overlay bit maps ROM at 0 as well as $400000 and moves RAM to $600000; software clears it after testing | III-39, III-42 |
| Keyboard serial protocol, commands and timing | III-29–33 |
| Mouse: motion interrupts through the SCC, direction and button through VIA register B | III-27, III-39–40 |
| Real-time clock and 20 bytes of parameter RAM through three VIA bits | III-36–38 |
| VIA register bits, timers and interrupt flags | III-39–42 |
| Interrupt levels: VIA 1, SCC 2, both 3, debugging switch 4–7; autovectors from $64 | II-196–197, III-46 |
| Disk geometry: 800 sectors of 512 bytes, 80 tracks in five speed zones of 12 down to 8 sectors per track | II-211 |
| A replacement ROM kit runs 128 KB ROM code on 128K and 512K boards; reflashing it needs more than 128 KB of RAM or an external programmer | SRC-0072 (third party) |

Volume III's hardware text calls the SCC a level-1 and the VIA a level-0
interrupt source (III-25, III-41), contradicting the same book's interrupt chapter
and vector table (II-196–197, III-46). This study follows the interrupt chapter,
which matches the vector table; owner observation can confirm it.

## RAM budget

Arithmetic from Figure 9 (II-19), the 16.75 KB typical system heap (II-9) and the
buffer addresses (III-19, III-21), for 131,072 bytes of RAM:

| Region | Range | Bytes |
| --- | --- | --- |
| Vectors and system globals | $00000–$00AFF | 2,816 |
| System heap (typical size) | $00B00–$04DFF | 17,152 |
| Application space, no alternate buffers | $04E00–$1A6FF | 88,320 |
| Main screen buffer | $1A700–$1FC7F | 21,888 |
| System Error Handler area | $1FC80–$1FCFF | 128 |
| Main sound buffer | $1FD00–$1FFE3 | 740 |
| Above the main sound buffer | $1FFE4–$1FFFF | 28 |

Using the alternate sound buffer cuts application space to 86,784 bytes; using
the alternate screen cuts it to 55,552. The application space also holds the
stack, application globals, 206 bytes of QuickDraw globals, 32 bytes of
application parameters and the jump table (II-19–20).

What this means for ClasSICK:

1. Applications read documented globals at fixed addresses and expect the same
   free memory, so the `system1-1984` profile keeps this layout.
2. ClasSICK's own RAM state (driver state, event queue, file control blocks,
   the native core's arenas) must fit in the system globals area and the system
   heap, as Apple's did. The system heap size is ours to set at startup (II-9),
   but every byte above 17,152 is taken from applications.
3. ClasSICK draws straight into the main screen buffer. The PC edition's 64 KiB
   staging buffer has no equivalent here; SPEC-0001's 1-bit MSB-first layout
   already matches the screen's bit order and 64-byte rows.
4. Code that does not fit in ROM must be loaded into RAM. Apple's fonts and desk
   accessories were resources read into memory from the system resource file
   (I-107, I-218), so ClasSICK equivalents loaded the same way cost what the
   originals cost. Toolbox and Operating System code that Apple kept in ROM has
   no such allowance.

## ROM budget

Apple's ROM fits in 64 KB (I-88). ClasSICK has three possible ROM capacities on
a 128K:

| Option | ROM space | Status | Consequence |
| --- | --- | --- | --- |
| A | 64 KB | Enough for Apple's ROM (I-88); SRC-0071 does not state how much a board decodes | All Toolbox and Operating System code in C must match hand-written assembly density. High risk |
| B | 128 KB | Vendor claim for 128K/512K boards (SRC-0072); not verified on hardware | Twice Apple's budget. Recommended target |
| C | Beyond 128 KB (the vendor's flash is 1 MB) | Not known whether a 128K board addresses it | Fonts, desk accessories and the desktop shell could run in place from ROM, saving RAM |

A replacement ROM is required in every option, because the M0.3 gate forbids
running Apple ROM code (`docs/architecture/boot.md`).

## Measured code size today

Indicative only: these use the Ubuntu cross compilers in Claude's workspace, not
the pinned Windows toolchain, and the objects were never linked or run. Source:
the current portable core (surfaces, arenas, input, clocks) and the boot scene at
public revision `1dd9fa6` (unchanged since `7eed40a`). Flags: `-Os
-ffreestanding -fno-builtin`; for GCC also `-mcpu=68000 -fno-pic`, for Clang
`--target=m68k-unknown-elf -mcpu=M68000`.

| Compiler and target | Code and read-only data (bytes) |
| --- | --- |
| GCC 13.3.0, MC68000 | 3,572 |
| Clang 18.1.3, MC68000 | 6,050 |
| Clang 18.1.3, i686 | 3,748 |
| Clang 18.1.3, x86-64 | 3,973 |

- GCC's 68000 output is 41% smaller than Clang's for the same code; R-011 should
  prefer GCC for size unless later measurements differ.
- No object uses an instruction beyond the 68000 (checked by disassembling our
  own objects).
- The 68000 has no 32-bit multiply or divide, so the objects call runtime
  helpers for multiply, signed and unsigned divide and unsigned remainder, and
  GCC also calls `memcpy`. ClasSICK must supply original, reviewed versions
  (architecture overview, compiler helpers).
- 3.6 KB is about 5% of a 64 KB ROM and covers no Toolbox code. The deciding
  numbers will come from QuickDraw and the managers as they are written.

## Startup path for the first 128K boot

Volume III documents what a first M0.3 boot needs: run from ROM with the overlay
set, test and size RAM (RAM repeats within its 4 MB block, III-17), clear the
overlay, set up the VIA and interrupt vectors, draw into the main screen buffer,
leave the sound buffer's low bytes to the disk motor (III-21), poll the keyboard,
and count frames from the vertical-blanking interrupt (III-18). Still needed before a
startup specification:

- the SY6522 VIA and Z8530 SCC datasheets as eligible sources (Volume III defers
  register details to them);
- SRC-0069 locators for the 68000 reset, exception and interrupt behavior;
- an eligible description of the floppy's encoding and the disk-speed values.
  Volume II gives only the logical geometry (II-211), so the first boot runs from
  ROM and the floppy driver is a later gate;
- the M0.3 test host: the owner's mini vMac choice must be checked for a 128K
  model that accepts a non-Apple ROM of the chosen size.

## Risks and unknowns

| Item | Effect | Resolution |
| --- | --- | --- |
| ROM space a stock 128K board decodes | Decides option A, B or C | Owner hardware check; vendor design notes; another published hardware source |
| C code density against Apple's 64 KB | Whether the Toolbox fits | Per-commit 68000 size report as managers are written |
| Speed of C QuickDraw on a 6–7.8 MHz 68000 | Usability, timing-sensitive programs | Measure in an emulator, then on hardware |
| System heap size in January 1984 | RAM left for applications | Owner observation (R-001) |
| Floppy encoding | Loading applications from disk | Further published source or black-box measurement |
| Reflashing needs another machine or a programmer (SRC-0072) | Development loop on real hardware | Emulator-first development |

## Recommendation

1. Target a stock Macintosh 128K logic board with a replacement ROM that
   provides at least 128 KB at $400000, and keep the documented RAM layout
   unchanged. This needs no new scope decision as long as the board addresses
   128 KB. If the owner's hardware shows only 64 KB, the charter's explicit
   scope decision applies: a reduced feature set, a board modification, or a
   Plus-class stage first.
2. Budgets: the whole ROM image at most 128 KB; ClasSICK's resident RAM at most
   the system globals area plus a 17,152-byte system heap.
3. Add a 68000 size report (GCC, `-Os`) to the build so every commit shows the
   ROM total; fail it when the total exceeds the budget.
4. Develop in an emulator first; real hardware is the final M0.3 evidence.

## Next steps

- Owner: say whether a Macintosh 128K or 512K, a replacement ROM kit or an EPROM
  programmer is available, and whether *Guide to the Macintosh Family Hardware*
  (Apple, 1990) or the SY6522/Z8530 datasheets are at hand.
- Claude: record the datasheets and SRC-0069 locators when available; draft the
  128K startup specification; propose the size report as an ADR with the
  original runtime helpers.
