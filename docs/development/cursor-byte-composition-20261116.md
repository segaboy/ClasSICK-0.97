# Byte-row cursor composition and refreshed Mac/286 review candidate

2026-11-16. Original implementation and BootHush observation, not historical
or physical validation. SPEC-0052 byte-row amendment / ADR-0076 / IMPL-0140 /
TEST-0198. This continues the owner's release and human-response work under
the current independent-implementation amendment. Codex authored and
self-reviewed the change; no subagents, external implementation sources,
reference ROMs, other emulators or WSL were used. Independent review remains
open. Active effort and provider usage were not measured.

## Change and selection

The Mono cursor path in `core/graphics/cursor.c` now saves and draws at most
three bytes per row instead of doing division, indexing and writes for each
pixel. Explicit byte assembly and unsigned shifts avoid native-endian or
alignment dependencies. Restore edge masks are computed once per rectangle.
Validation order, transparency, clipped margins, packed saved background,
state ABI and RGBA behavior remain covered by the independent pixel oracle.
No gain, zoom, quadrature polarity or foreground-input policy was changed.

The baseline is the previously selected 2048-record Mac build used for M1R4,
not the earlier frozen review package. The new native Mac and 286 images
include the same shared cursor implementation. The earlier archive and
[review record](../../releases/paired-native-preview-2-review.json) remain
unchanged. The current candidate record selects the following new bytes:

| Input | SHA-256 |
| --- | --- |
| Mac baseline ROM | `e6c0163867fe969075ccb4ef2dd9030830e1e9df9e5764476eda117027be1370` |
| New Mac ROM, 65536 bytes | `5d48f7877386e30f8e45e0456f528e93c1990aa7dd5b7ef37b40ddd7cf1de5a0` |
| New 286 disk, 21411840 bytes | `bacd3df9628136c493cbf6e5c630fec485546973c9c95567113fae9dfe740062` |
| New installer, 1228800 bytes | `c49f74e11a2f035f1145f5c922bf233808530a3ac1723529b3ae9c6fcb6ed1ef` |
| Unchanged BootHush executable | `fe262bc58ae58cb1d9b22ae5f409b71693e6ebf0fd305e4846692a5873588de8` |

The Windows-native Mac twins match. ROM usage is 49644 bytes (144 more),
workspace 41944 and native BSS 504 (unchanged), conservative summed stack
bound 4904 (20 less), largest individual frame 300 bytes. These are build
bounds, not complete 128K System or physical-machine certification.
The 286 payload is 22910 bytes, with no external runtime modules; the boot
sector and installer remain 329 and 1302 bytes respectively.

## Measured response

First matching function calls after the identical +64 X event at cycle
100000000 were bounded by entry PC and their stacked return PC. Both return
stack pointers were checked. Inclusive elapsed modeled time includes IRQs;
nested rows overlap, and interrupt phase can change even unchanged functions.

| Operation | Baseline modeled ms | Candidate modeled ms |
| --- | ---: | ---: |
| Restore saved cursor | 3.979 | 3.031 |
| Save/draw cursor | 15.787 | 4.558 |
| Complete cursor move | 23.256 | 11.367 |
| Scene compose | 27.681 | 15.503 |

The complete move takes about 51% less modeled time in this observation.
This is not a physical or host-to-screen latency benchmark.

`scripts/Test-MacCursorLatency.py` runs 36 cases on each image: four signed
64-pixel bursts and 32 streams of 64 signed one-pixel events at about 125 Hz,
covering eight starting phases and both axes. All 72 reach exactly the
requested position with zero losses, omissions, overflow, faults or pending
bridge input. All 36 candidate completions are earlier than their baseline.

| Workload | Baseline ms after last input | Candidate ms after last input |
| --- | ---: | ---: |
| X burst | 135.247-137.088 | 126.718-128.631 |
| Y burst | 135.071-135.858 | 126.647-127.435 |
| X streams | 62.279-68.974 | 51.580-55.226 |
| Y streams | 58.103-86.203 | 41.857-43.429 |

The diagnostic pointer is written after composition. Input delivery,
foreground work and interrupts all contribute. Axis timings still differ;
these results do not close the owner's perceived horizontal/vertical
response issue or establish device movement gain. The earlier phase-a
latency report preceded added total-loss/FIFO checks; final phase-b is the
selected report.

## Verification and artifacts

All directories below are under `C:/Repos/.task-temp/`. Reports retain exact
commands, input/tool/source hashes and outputs. The refreshed review archive
includes reports, the relevant committed sources, product-only captures and
the profiling/trace reproducers; verbose raw runs remain local.

| Evidence | Directory or file | Result |
| --- | --- | --- |
| Mac twin build | `mac-cursor-bytes-20261116-a/manifest.json` | Equal twins and size/stack checks |
| Function timing | `cursor-profile-baseline-20261116-a/report.json`, `cursor-profile-candidate-20261116-a/report.json` | Five matching entry/return pairs per image |
| Timing matrix | `cursor-byte-latency-20261116-b/report.json` | 72/72 exact, loss-free completions |
| Hosted checks | `cursor-byte-host-checks-20261116-a.log` | 9/9 cursor/scene/input and freestanding checks |
| 286 payload | `pc286-cursor-bytes-20261116-a/report.json` | Eight states, IRQ/configuration and pixel mutation checks |
| Media | `pc286-cursor-bytes-media-20261116-a/media.json` | Original boot/install media built |
| Media qualification | `pc286-cursor-bytes-media-checks-20261116-a/report.json` | 16/16 boot/install cases; eight installed native states |
| Paired scene | `cursor-byte-parity-2048-20261116-a/report.json`, `cursor-byte-parity-4096-20261116-a/report.json` | Eight states each; 175104 canvas and 48896 margin pixels exact; replay equal |
| Expanded boundaries | `cursor-byte-edges-20261116-a/report.json` | 14 exact state/pixel checkpoints |
| Excessive input | `cursor-byte-recovery-20261116-a/report.json` | Detected overflow cancels drag; fresh -16 motion recovers exactly |
| Recorded motion | `cursor-byte-human-traces-20261116-a/report.json` | Four replays pass |
| Presentation | `cursor-byte-captures-20261116-a/capture.json` | Eight paired checkpoints, installer captures and 13.1-second MP4 |

The independent cursor test adds 9792 show/hide scenarios over all 1-16
pixel widths, all horizontal hotspots, eight byte alignments, three layouts
and three clipping positions. Existing full-height, RGBA, move, alias,
refusal and saved-padding checks remain. Direct O0/O2 runs and an O2
AddressSanitizer/UndefinedBehaviorSanitizer run pass. The first sanitizer
invocation had a PowerShell comma-parsing error before execution; quoting
the sanitizer argument corrected it. The first CMake configuration lacked
Python; an explicit interpreter path corrected it. The selected build and
nine checks then passed. Unrelated Windows/viewer working edits are excluded
from the source package and were not used as qualification claims.

M1R2 accepted vectors replay at 1x, 2x and 4x with exact clamped positions,
zero losses/omissions/overflow and FIFO peaks 85/567/1285 (previously
120/637/1404). M1R4's actual accepted-vector trace reaches the same final
pointer and byte-identical final frame with peak 33 (previously 40).
Intermediate frame timing may change. These are replay checks, not new
human operation or raw physical-device measurement.

The excessive-input case intentionally overflows; it demonstrates recovery,
not lossless processing of that stress. The capture GIF presents completed
states, not synchronized platform timing. The MP4 is 131 guest frames at
10 fps with no audio; it decodes successfully and its final frame matches
qualification. The paired moved scene and verified installer result were
visually inspected. All art and guest captures are project-owned; no desktop,
reference firmware or proprietary assets are included.

## Human and release disposition

The [human receipt](guided-human-qualification-20261116.md) preserves all
owner feedback. P1 ended at its bounded limit without any accepted keys,
so it is unattempted for human controls. The earlier Space-toggle handoff
instruction was wrong for this raw-keyboard build: **hold Space to drag,
release Space to release; tap once in content to click**. BootHush's guide
now distinguishes it from the older BIOS diagnostic.

The new images need human Mac response/drag and 286 keyboard qualification.
Keep Mac gain 300% and zoom 2x for the next comparison. Startup-held and the
remaining cancellation/fault matrix, physical validation and independent
review remain incomplete. No owner waiver, publication, version assignment,
tag, complete game port or System release is inferred. A new source-matched
review package is appropriate; release readiness remains false.

## Completed package verification

The [refreshed package record](../../releases/paired-native-preview-2-cursor-review.json)
pins the archive and source revision `e21a295c204965f300f97ea370a94e602c0af425`.
Two independent preparations are byte-identical: 4875147 bytes, SHA-256
`c9b093a70bc0249860ef61d5a564d27cfc592761b3a2e49aba43cb4f4310a964`.
All 53 members and 80 committed build/preparation inputs verify. Altered ROM,
missing license, extra-file and altered-source controls are rejected.
The packaged Mac reaches stage 16 and the packaged 286 reaches POST B0.
Both GIFs decode (eight paired checkpoints and 32 retained port frames).
The source archive excludes unrelated working changes and private HQ content.

The repository guard passed after correcting an older cross-repository
relative link to an immutable BootHush source URL. Guard success is scoped
to indexed text, links, registered images, payload and common-secret checks;
it does not provide independent provenance review.

Local review folder:
`C:/Repos/Release-Review/ClasSICK-Mac286-Cursor-20261116/`.
It contains `ClasSICK-Mac286-Cursor-Review.zip`, the verified unpacked package,
`package-checks.json`, and `START-HERE.md`. M1R5 Mac and P1R1 286 launchers
were prepared with pinned inputs and complete five-minute recording; neither
was launched. Their output directories are outside the immutable package.
The Mac plan keeps unconfined relative input, gain 300% and zoom 2x.
The 286 plan uses the new raw-keyboard disk and the corrected hold/release
instructions. Original human recordings and the earlier review remain intact.
