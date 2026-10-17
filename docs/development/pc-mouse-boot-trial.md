# Native PC mouse boot trial - 2026-10-17

SRC-0238/0239; SPEC-0040 / IMPL-0044 / TEST-0049. The owner said "Continue"
after the proposed isolated PC boot/mouse test. This covered one fresh isolated
VirtualBox boot of the exact verified candidate. That session is now closed;
future trials require fresh owner approval. No runtime rebuild or release.

The compiled source is `59ea57f4b6db02ffb8ecd9cbd4df8f6c485448a9`;
the compiled-only receipt `db4151b670c2c4e781232a1b5cf9aa8c347d2231`
retains its dated no-target-run meaning. The native EFI SHA-256 is
`1f0d222b355f54b01a2822a8c857ba0f78b3dc9192744b562a6a3c2bdc2fd4c1`
(151,552 bytes). Two existing independent project writer builds produced identical
67,108,864-byte boot images; the separate reader and VDI/raw roundtrip passed.
Exact hashes and sanitized capture identities are in [the JSON record](pc-mouse-boot-trial.json).

VirtualBox7.2.16r174877 used a fresh one-CPU EFI64/piix3 machine with512MiB,
PS/2 keyboard/mouse and only our disk. All networking, USB, audio, clipboard,
file transfer, drag/drop and VRDE were off, with no shared folders. Existing
VM inventory and running states were preserved. Firmware remains an admitted
black-box platform; its self-report is not the missing B2 criterion6 image hash.

The real native scene reached READY: APP/097, branding, hollow rectangle and six
phase marks, monochrome512x342 centered at(256,213) in1024x768. Both A20 probes
returned PASS and controller output-port values stayed03/03. These are scoped
prototype observations, not historical Macintosh evidence or edition acceptance.

| Input/control | Observed combined counter | Disposition |
| --- | --- | --- |
| Initial scene | 0000 | READY and drawing observed |
| Two desktop automated clicks, one drag, one Space | 0000 | Desktop delivery unqualified |
| Direct VirtualBox Space39/B9 | 0001 | Synthetic keyboard path passes |
| Public IMouse relative left press | 0002 | Synthetic PS/2 mouse path passes |
| Repeat while left remains held | 0002 | No duplicate press credited |
| Release, second press, release | 0003 | Second mouse edge credited |
| Normal terminal result at guest second60 | 0003 | result0 / keyboard-ready1 |

Mouse injection used the installed public COM interface, whose method signature
was verified by introspection: relative dx/dy/dz/dw all0, button masks0,1,1,0,1,0.
No emulator source, firmware memory or implementation was inspected. The UART
carrier's legacy `space=` label means combined input under the compiled v2 profile.
No human-input confirmation was received; no UI transport, physical input or
universal controller compatibility is credited. Cursor motion/UI and Event Manager
parity remain open. The native PC OS code ran directly as x86, not through68k.

Seven direct framebuffer captures preserve every pixel outside the defined progress
and counter rectangles; the rectangle/branding/result panel remain exact. Held-repeat
counter pixels are identical. The session was paused/resumed for inspection, so this
is not an uninterrupted wall-clock timing trial. The first log read collided with
exclusive file sharing; shared-read access corrected it. One early framebuffer
capture returned0x0; the same boot later exposed its display. A first window capture
was occluded; activation and direct framebuffer captures resolved observation.
All issues are retained, without another cold start or a hidden successful retry.

No new worker call was needed for target control/final verification. Existing2,925
CI tests are reused, not rerun or counted as this trial. No code, UART source/test
body, Mac ROM, research original/copy, old evidence row, release or tag changed.
OneNote/wiki maintenance remains retired. The current Mac native profile still
needs its own fresh approved observation; both System1/ROM boot requirements,
historical/physical/full128K/B2 and full native feature parity remain open.

Next: real operator mouse validation when available, then separately scoped paired
native pointer/event or drawing-manager work. This record authorizes neither a
new target run nor new runtime implementation.
