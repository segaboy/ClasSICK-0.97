# Loaded drawing ROM: isolated Snow trial - 2026-10-16

SRC-0207 / SRC-0208; SPEC-0036 / IMPL-0040 / TEST-0044.

One freshly authorized original-ROM Macintosh 128K cold boot displayed APP/097,
the application's hollow rectangle and all six completion marks. The kernel
counter later advanced from 0000 to 0001. The result panel and phase marks are
pixel-identical at those snapshots and at a later 0001 snapshot. This is a
scoped boot and retained-drawing observation, not complete mouse qualification.

## Exact candidate and conditions

- Compiled source `7469caaf43346b019f98d75edcfaa7b3e2007917`; compiler/CI checkpoint
  `1ffc49410af96124d85edf68ce9d5d4aa6a08582`. No rebuild or native input changed.
- Original ROM: 65,536 bytes, SHA-256 `494aa494b09ff616b8c755b5a07d61dc84594990336d718b65295e611d7d895a`.
- Snow v1.5.0-317c188 Windows x86-64, SHA-256 `32cf5285582c0f509e4f1ac38d86bfb90440bae9da43b7823bfb0841dea588e6`;
  previously admitted black-box platform SRC-0082, no emulator-source reading.
- Early128K model, normal speed, RelativeHw mouse, separate fresh workspace and
  profiles. No Apple ROM, disks, saved machine state, shared directory or serial
  bridges. Workspace at launch SHA-256 `1384b8e6075c5d114f35d3cf87658d525e7585667b112f564751db654ca8411c`.
- One logged guest reset at 2026-10-16T14:08:43Z. The first sandboxed launcher
  exited before a window or guest reset; the same isolated workspace was then
  launched outside that process restriction. This is one cold boot, not two.
- Normal close logged at 2026-10-16T14:12:48Z; process/window gone.
  Future emulator/VM/reference trials need fresh explicit owner go-ahead.

## Captured observations

- 2026-10-16T14:09:14.565Z: complete-boot, counter 0000.
- 2026-10-16T14:11:15.799Z: clean-counter-snapshot, counter 0001.
- 2026-10-16T14:12:32.865Z: later-stability-snapshot, counter 0001.

Private PNG originals and raw logs remain outside Git. Their sanitized identities:

| Capture | Bytes | SHA-256 |
| --- | ---: | --- |
| boot-0000.png | 75297 | `6abf839f4b51e35e3121191d5b320e7118e19ce3520100f5796967c9dec0809b` |
| counter-0001-menu.png | 78132 | `32eeb35fecb838a3bb62ad7e50f85bc88bcaafa951e7c21afa991b4a58d7f9d7` |
| counter-0001.png | 75637 | `2a8fa1d0fab8183b0700f28704606c88c33c181367f799c4f1bdba93f3356c3f` |
| counter-0001-stable.png | 75512 | `6e6a3db3f155bba17dd2f25441bc9895cb61b1fcad5ffc2023bb44dda3b594c3` |

The fixed 1002x782 window snapshots have identical RGB pixels in screenshot
rectangle x=[90,912), y=[340,550), expressed exactly as half-open crop coordinates
`(90,340,912,550)` (x1,y1,x2,y2). This region contains the native result panel,
the hollow rectangle and six phase marks. Comparison is point-in-time evidence;
it is not continuous frame tracing or a calibrated timing measurement.

## Input and qualification limits

An automated left click, short drag and right click were not immediately
visibly credited. A subsequent capture showed 0001, so the counter advances,
but exact one-to-one event attribution and delivery reliability are unresolved.
A transient host window menu was dismissed before the clean 0001 capture.
The owner answered "I’m unavailable—record the automated result". No human
input is credited, and no keyboard, click-rate or latency claim follows.

No Apple or Microsoft implementation, manual sample, external ROM/media or
emulator source was used. No RAM or disassembly diagnostic window was opened.
No general QuickDraw/guest GrafPort/region/callback/native-app claim. Historical
R-001/AL-2/QQ-1..13, source/UART, physical/full128K/parity/edition/B2/human gates
and both mandatory System1/ROM interoperability targets remain open. The
published alpha assets and tag are unchanged; this candidate is unreleased.
