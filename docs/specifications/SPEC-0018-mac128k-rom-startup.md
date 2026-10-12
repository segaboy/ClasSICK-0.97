# SPEC-0018 - Macintosh 128K original ROM demonstrator

Version 1, 2026-10-11. Accepted for development and the owner-directed Snow
trial (SRC-0076). This is an original startup profile, not System 1 API parity
or completion of M0.3. Implement/test under IMPL-0022 / TEST-0026; ADR-0021.

## Documented hardware interface

SRC-0080 is the published 1985 Inside Macintosh Volume III, eleventh printing
June 1990, independently checked at the following printed pages. The facts
describe 128K/512K hardware; later-manual System 1 API confirmation remains open.

| Fact | Locator |
| --- | --- |
| ROM at 0x400000; reset overlays it at zero, RAM temporarily at 0x600000 | III-18, III-42 |
| Main screen on 128K at 0x1A700, 512 by 342, 64-byte rows, 1 bit black, MSB first | III-19 |
| VIA uses even-addressed byte accesses; direction bit 1 means output | III-39 |
| A bits 6/4/3 select main screen/overlay/main sound; B bit 7 disables sound, bit 3 active-low mouse | III-39-40, III-43-44 |
| VBL flag bit 1 is set even with its interrupt disabled; approximately 60.15 frames/s | III-18-19, III-41 |

SRC-0078: Synertek Programming Reference, September 1980, internal register
summary (PDF page 2, printed 3), port direction (PDF page 3, printed 4-5),
interrupt registers (PDF page 6, printed 10-11); AN5 (1982) section 1/Figure 1
(page 2) independently confirms register-1 read clears CA1 flag and IER disable
leaves flags active. Register 15 has no handshake. SRC-0079: Motorola MC68000UM
6.2.1/6.2.4/6.3.1/6.3.5, printed 6-4/6-9/6-11-13: reset fetches SSP/PC at
zero/four, supervisor state with mask 7, and ordinary TRAP return via RTE.

## Own memory and reset contract (design decisions)

The ROM is 65,536 bytes, padded deterministically, loaded only at the normal
ROM base. The first 256 bytes contain 64 big-endian vectors: initial SSP
0x4000, initial PC in our ROM, own fault targets, and a dedicated TRAP #0 probe
handler. A four-byte image seal at offset 0x100 contains CRC32 of the complete
image with that field treated as zero. This is a corruption/admission check,
not cryptographic proof or verification of an emulator's implementation.

Execution remains in ROM while the overlay is cleared. Startup uses no stack
before RAM is available. Port A output latch 0x48 and direction mask 0x58 drive
only main screen, main sound selection and overlay, leaving disk/volume/SCC
bits inputs. All subsequent exception vectors are copied to RAM at 0-255.
No Apple initialization or firmware call occurs.

Own initialized data/BSS starts at 0xB00 and ends before 0x1000; a reserved
stack occupies 0x2000-0x3FFF. The main framebuffer occupies 0x1A700-0x1FC7F.
Startup copies only own .data, zeros own BSS, verifies the loaded image, and
executes one original TRAP #0 probe which increments an own diagnostic counter
and returns. No historical low-memory globals, trap dispatch table, application
heap, disk driver or System/Finder image is provided.

## Registers touched

Addresses derive from III-43-44 and the Synertek register numbers; the code
contains original declarations rather than copied equate files.

| Address | Register | Access/purpose |
| --- | --- | --- |
| 0xEFE1FE | B, register 0 | Set sound-off latch; poll mouse bit 3 |
| 0xEFE3FE | A, register 1 | Read to acknowledge VBL |
| 0xEFE5FE | DDRB, register 2 | 0x80: sound off output; mouse/RTC inputs |
| 0xEFE7FE | DDRA, register 3 | 0x58: selected startup control outputs |
| 0xEFF7FE | ACR, register 11 | zero: input latches/shift/timer output modes off |
| 0xEFF9FE | PCR, register 12 | zero: falling-edge CA1, other control inputs |
| 0xEFFBFE | IFR, register 13 | Read VBL flag only |
| 0xEFFDFE | IER, register 14 | Write 0x7F to disable all VIA interrupt delivery |
| 0xEFFFFE | A, register 15 | Write/read startup selectors without handshake |

The CPU stays supervisor with interrupt mask 7. No timer, SCC, IWM, keyboard
shift register, RTC command, sound-buffer byte or disk motor register is written.
The selected CA1 edge is an explicit project choice to qualify in the emulator.

## Scene, time and input

Use SPEC-0001's MONO1_MSB surface directly over the main screen. Draw an original
geometric panel, progress segments and mouse-change indicator; original small
glyphs label the project and counters. No Apple fonts, icons or pixels.

The scene validates its full surface descriptor through SPEC-0001, then fills
its known bounded rectangles a byte at a time, masking edge bits and preserving
row padding. This original specialized renderer avoids per-pixel address
multiplication on the 68000. Five whole-frame baselines from the earlier original
pixel renderer verify unchanged output, including input parity and completion;
padded rows and invalid descriptors have independent boundary checks. Shared
portable/PC graphics source remains unchanged.

After initial drawing, synchronize to one VBL flag and acknowledge it. Start
progress at zero; count 3,610 further flags before completion (over 60 seconds
at the documented approximate frame rate). Each flag is acknowledged by a
register-1 read. Polling may miss coalesced flags while drawing, extending elapsed
time; never present this as calibrated wall time. A stopped VBL leaves progress
unchanged. Report host elapsed time separately in any observed trial.

Poll mouse bit 3 on every loop. Count a released-to-pressed transition once,
change the panel's pattern, and retain the count; a held button never repeats.
Ignore an initially held button until release. Continue accepting mouse input
after progress reaches completion. No cursor motion or keyboard protocol yet.

## Failure and verification

Image-seal mismatch stops before the normal scene. CPU exceptions take our
terminal handler, set an own diagnostic fault value and loop without returning.
Rendering errors similarly record a fault and stop. No boot-service fallback.
Diagnostics are an own volatile record in BSS (magic, stage, flags counted,
presses, TRAP count, calculated seal and fault); publish only own output.

Independent tests cover arithmetic helpers against host-width oracles, image
CRC/corruption, screen guards/pixels, mouse edges, VBL progression/stall and
completion saturation with injected register callbacks. An unloaded ELF/ROM
audit checks vectors, spans, helpers, absence of undefined/runtime imports and
determinism at Os/O0 with 68000 assembler enforcement. The trial records exact
source/tools/image/platform/settings, cold-start screenshots, elapsed timing
and owner input confirmation. Emulator execution is not real hardware evidence.

Compiler helper contract: original 32-bit multiplication modulo 2^32, unsigned
quotient/remainder and signed division truncated toward zero. Signed minimum
divided by -1 returns the minimum bit pattern by design; zero divisor triggers
our fault path. Original memcpy handles non-overlapping valid ranges only.
No target libgcc/libc runtime is linked. Unneeded 64-bit helpers are not adopted.
