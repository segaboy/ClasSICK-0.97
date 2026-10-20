# Pointer/motion source and input evidence inventory -2026-10-19

SPEC-0051 / ADR-0059. This is an inventory and proposed qualification path,
not a new target observation or hardware motion implementation.

## Mac source lane: owner-cleared review; adapter prerequisites remain open

SRC-0314 records the owner's "Continue the narrow published-source review".
The previous source-review hold is resolved; the unrelated declaration remains
excluded, with no affected implementation or worker exposure. The historical
held checkpoint is preserved.

[The completed visual source assessment](mac-pointer-source-review.md) verifies
SRC-0080 identity and mouse prose/table/Figures5-7 at III-27..29 (PDF34..36).
It establishes published signal wiring and direction facts. It does not establish
complete SCC initialization/status/acknowledgment, VIA ownership, CPU routing,
loss/baseline/coherent delivery or target cost/applicability. Gaps M1-M7 block
Mac adapter admission. No decoder, controller algorithm or polling procedure is
inferred. Printed symbolic declarations/sample code stay excluded from adoption.

The first contract/inventory slice is complete as an assessment with blockers;
it is not a complete hardware specification. The next recommended engineering
slice is the cursor composition contract. No private PDF/text/page image is
committed or supplied to external workers. No new implementation or run occurs.

## Existing native input evidence

| Evidence | Established result | Does not establish |
| --- | --- | --- |
| SPEC-0039 shared button / pure PS2 decoder | Original edge/saturation/drop rules;PACKET returns raw signed dx/dy/buttons | A complete moving-pointer/cursor path or native directional convention |
| SPEC-0040 coordinator | Single controller owner,ACK/report routing,cancel/rebaseline on corruption in its current profile | GUI host capture/focus or the new SPEC-0051 binding |
| Hosted Windows viewer dispatch | Existing hidden-window button/capture-loss/cancel tests pass | Native PC VM input or physical/human pointer motion |
| Paired preview1 Mac | Exact ROM in Snow Early128K,human2 clicks and intact drawing | Mac motion,full128K System feasibility,physical Macintosh,mini-vMac |
| Continuous PC own-image session |38 mouse+5Space injected through185 guest seconds;static scene preserved | GUI capture/focus or human reliability;this is E2 device/API injection |
| PC human report | Intermittent clicks after many presses;qualification failed/unreliable | A known decoder bug,or a later human pass |
| GUI startup attempt | VERR_CPUM_TOO_MANY_CPUID_SUBLEAVES before guest initialization;unchanged-VM headless fallback passed | Root cause,a host/OS fix,or successful GUI operation |

The previous185-second session also slightly exceeded its noted180-second
total-active budget. A future harness needs one shared outer host deadline;
the guest clock and individual waits cannot independently extend it.

## Proposed qualification protocol: UNEXECUTED,not authorized to run

1. Freeze the future exact ROM/EFI/disk hashes,compiler source,contracts and declared
   coordinate transform. Obtain the owner's go-ahead for each new exact-image trial.
   Current release assets remain untouched;no Apple/reference media are involved.
2. Use the already admitted Snow Early128K and VirtualBox Windows11 profiles only.
   Fresh own workspace/VM,own media only,no network/shares/clipboard/USB/bridge;
   preserve prior inventory. Do not alter host firmware/security/CPU settings.
3. For each separately authorized target session,propose one GUI launch and one
   outer180-second host budget from launch through input/capture/cleanup observation. Abort on the predeclared
   deadline;cleanup must still close owned sessions. A GUI preboot failure is
   recorded and stops E3;headless fallback cannot silently qualify GUI delivery.
4. E1:hosted core/decoder/owner tests with independently authored original traces,
   exact state/flags,bounds/cancellation and later cursor-pixel oracles.
5. E2:device/API injection validates guest routing and logical results. Label its
   counter/motion/pixel results E2,even when a display window happens to be visible.
6. E3:deliver bounded host OS mouse movement/press/release through the visible
   target window. Record host focus,capture state,window scaling and guest/logical
   transform. Establish the starting coordinate/count after capture setup;that
   first click can count. Compare requested inputs to actual delivered/accepted
   transitions,including release outside the content and capture-loss cancellation.
7. Future motion/drag cases must visit edges/corners,move and release outside the
   frame,leave no trails and preserve the rest of the logical scene/PC margins.
   End with a declared recover/cancel sequence. Do not replace a two-event check
   with uncounted repeated clicks or infer timing precision from requested delays.
8. E4 is a separately logged human session when available and authorized.
   E1/E2/E3 never become E4. Human failure remains a failure;unavailable is not a pass.
9. Save only reviewed own-product framebuffer evidence to the public screenshot
   manifest,with hashes/captions/source pins. Manual pages/private UI are excluded.
   Close owned sessions and verify original inventory/running state is restored.

These are planned checks,not executed tests or proof that the PC capture issue
has been fixed. Frozen release1 limitations are not blanket waivers for release2.

[Existing PC receipt](preview-session-evidence.json),[human observations](paired-preview-boot-trial.md),
[current published pair](../../releases/paired-native-preview-1-publication.json),
[release2 plan](../../releases/paired-native-preview-2.md).
