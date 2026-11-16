<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Paired preview qualification work

2026-11-16; continuation from `8b72181`. SRC-0608 / SPEC-0109 /
IMPL-0138 / TEST-0196. Codex implementation and self-verification under the
independent-implementation policy; no independent human review asserted.
No Apple/Microsoft implementation, firmware, game code or new third-party
implementation input was used. Existing unrelated edits remain preserved.

## Buffered Mac input

The new `--window-buffered-collect` build option captures into a 128-entry
FIFO in the interrupt and decodes in foreground. Packed aligned samples and
explicit five-word copies avoid odd accesses and the byte-copy runtime cost.
Foreground owns both normalized inbox sides. FIFO overflow cancels input and
requires a fresh baseline; it cannot masquerade as lossless motion.
The original full-register interrupt veneers remain selected in the retained
build. C-ABI-save experiments were measured but are not required by it.

Retained twin build:
`C:/Repos/.task-temp/mac-buffered-20261116-f/manifest.json`.
ROM SHA-256: `baa542304ec55bcfd2fecae7c594bb8e161275e49f6c3145886f2ab2f52f75be`.
The 65,536-byte image uses 49,496 ROM bytes, 3,084 data/BSS bytes, 964 workspace
bytes and a conservative 4,924-byte stack bound within the 24,320-byte reserve.
Its manifest identifies all source/specification/tool inputs and exact three
interrupt veneers. The frame-alignment audit covers service and both decode
functions; deliberate odd accesses are rejected.

The existing BootHush build is
`C:/Repos/.task-temp/boothush-pc-after-active-20261116-a/a/boothush-active.exe`,
SHA-256 `8860227d991a6c6d252815019c2458a1b0af3cae49306ce43e7fdaff5dd04e3b`.
This is the active-profile tool build with the paused x64 archive retained,
not a source-complete BootHush distribution. No other emulator or WSL ran.

| Same +32 X test | Prior image | Buffered image |
| --- | --- | --- |
| Edge 2,048: final X / omissions | 254 / 14 | 288 / 0 |
| Edge 2,048: maximum complete IPL7 span | 4,738 cycles | 2,308 cycles |
| Edge 4,096: final X / omissions | 288 / 0 | 288 / 0 |
| Edge 4,096: maximum complete IPL7 span | 4,738 cycles | 2,340 cycles |

These are BootHush observations, not physical timing. Trace/plain output and
complete frames match. Commands and complete witnesses:
`C:/Repos/.task-temp/mac-buffered-mask-20261116-f/report.json`.
Four production SCC boundaries preserve D0-D7, A0-A7, exception stack, saved
SR and return PC; 18 deliberately corrupted comparisons are rejected:
`C:/Repos/.task-temp/mac-buffered-preservation-20261116-f/report.json`.

Both the formerly failing edge-2,048 case and edge-4,096 case now pass all
eight natural shared-window states, all 175,104 canvas pixels and 48,896 white
EGA margin pixels, plus Mac cycle/memory/frame replay. Reports:
`C:/Repos/.task-temp/mac-buffered-parity-2048-20261116-f/report.json` and the
corresponding `mac-buffered-parity-4096-20261116-f/report.json`.
The paired 286 image remains the independently built native diagnostic from
`pc286-pc-after-session-20261116-a`; these tests do not promote its loader.

Twenty-nine rebuilt existing motion/inbox checks pass. The new FIFO/deferred
decode tests pass 12,132 assertions in each of O0/O2; they include every word,
wraparound, exact capacity, overflow, saturation, delayed hardware mismatch,
late-latch handling and cancellation/rebaseline. Logs and identities are in
`mac-buffered-host-tests-20261116-f.log` and
`mac-buffered-host-20261116-f/report.json` under `C:/Repos/.task-temp`.

The native excessive-burst test deliberately requests +5,000 X while held.
It observes queue peak 128 and 22 queue overruns, first loss reason 5, release
without a stuck drag, and a subsequent exact -16 X move with the released
window stationary. There are 43 total decoder losses in this stress case;
it is explicitly not lossless. Fault remains zero. Reproduction and captures:
`C:/Repos/.task-temp/mac-buffered-recovery-20261116-f/report.json`.

Earlier attempts remain outside Git: XOR-only arithmetic (4,682 cycles),
packed-only (4,874), split READY paths (4,262 at the slower interval), and
buffering with aggregate copies. The latter still called a byte-copy helper
for 1,558 cycles inside a 3,420-cycle SCC span, identified in
`mac-buffered-attribution-20261116-b.json`. Removing that copy and the obsolete
foreground inbox exclusion is the material architectural change. Failed
attempts are not passing qualifications; the new API is opt-in.

## Preview boot disk and installer

SPEC-0110 / IMPL-0139 / TEST-0197 add original assembly for a 329-byte boot
sector and 1,302-byte installer. The existing native window payload remains
22,382 bytes with 21,936 BSS bytes, ending at near offset 44,318; the stack
starts at 65,534. This is a linked separation budget, not a measured worst-case
stack bound. The installer staging ends at 27,136 below scratch at 28,672.
No external runtime modules are linked. The supported disk model is exactly
615/4/17, 21,411,840 bytes, owned in full without a partition table/filesystem.

The installer checks the staged prefix, asks for INSTALL, scans the entire
target for zeros, writes/verifies payload sectors and commits the boot sector
last. The loader checks memory/geometry, retries reads and verifies CRC before
entering the unchanged payload. These are original preview paths qualified
in BootHush's BIOS model, not physical installation guarantees.

Retained build: `C:/Repos/.task-temp/pc286-preview-media-20261116-c/media.json`.
Hard disk: `6f17583422edf2eddd4e5c325020a3538b4333a0a3305ca350b99cf91e992ba6`.
Floppy: `399d69b06a5fc231a47a08118440a71caf6d707ecb21a8054c9946b983c7bc6b`.
Payload: `bb0f402bbf7b9d22d7999f7afc6e44f8defdc74de4acc012af2f349818dfd336`.

`C:/Repos/.task-temp/pc286-preview-qualification-20261116-b/report.json`
passes 16 media cases: prompt, cancel, installation, installed boot, occupied
first/last byte, corrupted staged data, persistent payload/boot-sector write
failures, preflight read failure, transient write/read recovery, bad payload
CRC, first/final payload-sector read failures and boot read retry. Failure
injection counts are checked. Refusal images stay unchanged; successful
installation is byte-identical to the complete prepared disk. Installed boot
reaches POST B0 at 7,205,065 modeled cycles. All eight native interaction
states match their prior independently-oracled frames and memory. Prior
payload mutation/IRQ checks are explicitly inherited, not represented as
fresh media tests; this loader clears BSS without the old diagnostic poison.

Both final disk/ROM comparisons pass all eight states at edge intervals
2,048 and 4,096, with zero omissions/losses/faults, exact 175,104-pixel canvas,
48,896 white margin pixels and cycle/memory/frame replay. Reports are
`paired-preview-parity-2048-20261116-a/report.json` and the corresponding
4096 directory under `C:/Repos/.task-temp`. These supersede the earlier
diagnostic-disk pairing for the prepared package.

`paired-preview-edges-20261116-c/report.json` adds 14 checkpoints over 172
samples on the same images: four-corner drag/clamp, release, cursor restoration,
content hover and three press/release pairs with repeated keyboard makes.
An independent scalar oracle checks state/counts; shared hosted rendering and
both native frames match at every checkpoint. Mac input uses relative motion
at edge 2,048. Earlier -a rejected out-of-canvas window-message arguments;
-b exceeded the 1,024 scheduled-input limit in a longer PC sequence. These
were harness refusals, not passing guest checks; the bounded -c is retained.

## Capture evidence

`scripts/Capture-PairedPreview.py` generated
`C:/Repos/.task-temp/paired-preview-captures-20261116-a/capture.json`.
The GIF shows eight matched checkpoints at one second each; it does not
represent continuous synchronized platform timing. The MP4 is an actual
continuous Mac guest-time capture: 131 samples at 10 fps, 13.1 seconds, no
audio. Its final frame exactly matches qualification. FFmpeg decoding passes;
the paired and installer PNGs and a decoded video frame were visually
inspected. This is author inspection of emulator output, not E4 human input.

BootHush's original diagnostic lettering enables readable installer/error
captures without a firmware font. Font SHA-256:
`0d2224d4908bd4856e001b1e7aeb2409b92ebd785d5691d41db6253759ea0517`.
It has deliberately limited glyph coverage and is not historical EGA artwork.
The port separately rendered its already-qualified recovery-jump samples;
that prototype is not part of the ClasSICK binary pair.

## Release disposition

The review package uses the fixed-profile preview loader and installer above.
The Mac/286 current preparation record is
[`releases/paired-native-preview-2-mac286.json`](../../releases/paired-native-preview-2-mac286.json).
The old x64 plan remains historical. The fast-input regression, original
installation path, modeled parity and review packaging are now concrete.
No public version/tag or published immutable prerelease was changed.

Open: actual GUI-host input and human disposition, physical Mac polarity/full
128K/PC hardware qualification, independent source/tool/provenance review,
startup-held and full target cancellation/fault acceptance, and the existing
historical/edition/System interoperability gates. The sustained stress test
above covers one cancellation/rebaseline path, not every failure mode.
The owner must explicitly dispose of unavailable criteria before declaring
this bounded experimental preview release-ready. Publication is separate.
