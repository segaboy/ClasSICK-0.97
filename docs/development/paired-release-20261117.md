# Paired experimental release completion, 2026-11-17

SRC-0611 / TEST-0199. The owner explicitly directs finishing the paired
release without further human testing and advancing BootHush and the port in
support. The current [release decision](../../releases/paired-native-preview-2-release.json)
authorizes ROM 0.4.0-alpha.1 / PC 0.2.0-alpha.1 as a bounded experimental
preview. It supersedes further-owner-test and separate-publication blockers
for this preview. It does not convert missing/failed qualification into passes.
Frozen review files and previous releases remain unchanged.

## Automated qualification

The exact reviewed Mac ROM, hard disk and installer remain unchanged.
The Mac hash is `5d48f7877386e30f8e45e0456f528e93c1990aa7dd5b7ef37b40ddd7cf1de5a0`;
disk `bacd3df9628136c493cbf6e5c630fec485546973c9c95567113fae9dfe740062`;
installer `c49f74e11a2f035f1145f5c922bf233808530a3ac1723529b3ae9c6fcb6ed1ef`.
All final target runs use BootHush
`9dc2f28425bafd10114cfde9ab5c439d61234873854cfa1c211cdcfb26bda82d`.

Results are retained at `C:/Repos/.task-temp/paired-release-20261117/` and
selected reports are included in the final distribution:

- `input-profile`: 46 checks pass. Single-axis bursts, eight phase offsets of
  streamed input and diagonal streams produce exact coordinates with zero
  loss, omissions, overflow, pending input or fault at edge spacing 4096.
  M1R5 replay reproduces 27 omissions at 2048 and zero at 4096. Both endpoints
  and frames match; checking only the final screenshot would miss the loss.
- `pc-native`: eight native scene states pass independent pixel comparison;
  original arithmetic helpers close the runtime with no linked compiler
  runtime modules. Two pixel-defect controls are rejected.
- `pc-media` / `pc-media-checks`: a fresh build reproduces both reviewed media
  hashes; all 16 install, occupied-target, corruption, cancellation, retry and
  boot checks pass, plus eight native states on the resulting disk.
- `parity4096-final`: eight checkpoints match the complete 175104-pixel
  shared canvas and 48896 white PC margin pixels. Mac record/replay cycles,
  memory and frames match; losses, omissions and faults are zero.
- `captures`: paired screenshots, an eight-state checkpoint GIF and a
  continuous sampled Mac clip are generated from those passing identities.

One initial parity invocation correctly refused the old PC report because
its emulator hash differed. The PC payload/media and their qualification were
regenerated with the exact new executable before the passing comparison; the
identity guard was not relaxed. Earlier 14 corner/clamp checks, host cursor
oracles and overflow-recovery results remain in the frozen evidence with
their original executable/rate identities, not relabeled as new runs.

The release profile is **edge 4096 / inversion 3 / VBL guard 4096**. This is
a measured model configuration, not a repair or qualification of host pointer
gain or physical mouse latency. No guest counter or decoder was altered to
hide omissions. The 2048 trace remains a required negative control.

## Human session disposition

M1R5 remains a failed relative-speed/axis observation. M1R6 was interrupted
during execution review; incomplete artifacts remain. The telemetry-disabled
M1R6b launcher subsequently completed its 300.156-second idle run with unchanged
inputs and zero accepted mouse/key input. It is not a human pass. The local
workflow is closed rather than leaving an active testing handoff. The owner
now asks for automated completion; no further desktop test is required for
this experimental publication. Physical/full128K, broader input cancellation,
historical identity, System interoperability, native-app, Linux and independent
human provenance/toolchain reviews remain open.

## Supporting projects and evidence transfer

BootHush commit `f7aa60b683de85b571ca01ce45966ca61f43f1c2` adds two original
mouse-timing regressions: 28 headless boots cover normalized axis-swap pacing,
queued vectors, VBL guard crossing, replay and observer equivalence. Opposite
axis polarity gives an expected first-DCD phase offset; equal pacing does not
mean equal first-interrupt times. These checks isolate the bridge from guest
capture omissions and support keeping 2048 outside the release profile.

Not Super Tilt Bro commit `cf3c29bac244efd21c49b4da07313087b2d206e6` adds
seven title-recovery runs with 13 checkpoints, record/repeat/replay witness
equality and a rejected held-click defect. Existing 22 title scenarios and
160 axis cases also pass. Its unchanged ROM hash is
`c693dcfebf1c9c4b1048abc50503f3462985a926dedc57fe6c5d1da25bfb7d72`.
The transferable evidence is the retry/stale-input test scenario, not game
implementation or a claim that ClasSICK implements that recovery. No game
code, NES assets, external firmware or protected reference bytes were imported.

## Packaging and provenance

The new release builder preserves the frozen review, verifies the three image
identities and all 80 frozen source-input hashes against the retained build-source
archive, checks image-producing inputs against current committed source, includes corresponding
source and GPL notices, and emits a complete SHA-256 inventory. Current release
instructions replace the pending review handoff in the new package only.
The verifier checks archive/directory membership, source/license correspondence
and authorization metadata; tamper controls are separate from guest tests.
Final remote release IDs, asset hashes and download verification belong in the
publication receipt after publication, not in a prematurely successful claim.

Codex authored the input-profile harness and release records from project-owned
source, existing specifications, retained original traces and the owner decision.
Delegated Codex work supplied original BootHush/port tests and packaging helpers;
root reviewed returns. No Apple/Microsoft implementation, held UART source,
untracked research originals or private HQ content was supplied to workers.
Project contributions remain GPL-3.0-or-later. No external implementation or
runtime was added. Author source/rights review and repository guards do not
substitute for independent human provenance review. Exact worker elapsed times
and provider usage are unmeasured.

## Published outcome

Both annotated tags point to `22a7e2dea0d452b620d5c6bd8cfc820c975972e6`.
The [publication receipt](../../releases/paired-native-preview-2-publication.json)
records both public experimental releases and all 16 downloaded asset hashes.
The 74-file paired package is 9,390,175 bytes, SHA-256
`4436d2a449e2273616446cdcd9324d6a413dd4a108c26e1899a9cb06996fc21d`.
Two preparations are identical; 30 tamper controls reject. Eight decoded GIF
frames match the PNGs and all 131 MP4 frames decode. The builder replaces a
filename suffix when choosing its ZIP name; the external ZIP was renamed to
retain both complete version strings, without changing bytes. Prior releases,
frozen reviews and unrelated dirty work remain unchanged.
