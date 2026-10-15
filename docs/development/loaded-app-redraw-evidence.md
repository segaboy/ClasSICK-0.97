# Loaded application redraw refinement - 2026-10-15

SPEC-0035 / IMPL-0039 / TEST-0043 / SRC-0193..0195.

The native overlay uses the project's original masked byte fill instead of
per-pixel generic fills. The kernel still polls and records ticks, but only
redraws the four-digit mouse counter when presses changes. APP/097, the captured
guest pattern, phase marks and every unrelated pixel remain in place. No CPU,
loader, application CODE, shared graphics or previous ROM-profile source changes.
This is original native presentation, not guest QuickDraw or Event Manager work.

## Completed local checks

Before editing the renderer, an original harness at `1b455ea24a163842e409b5915ccea9e07ae1e870`
captured12 complete frames covering success/failure, phases, patterns and counter
rollover. All262656 output bytes match the optimized renderer byte-for-byte.
The harness, frames and identities stay outside Git;12 frozen FNV1a32 frame
fingerprints enter the regression. Incremental sequences match full frames,
including repeated/decreasing counters,9999->10000 and UINT32_MAX. Random and
all-ones backgrounds verify untouched pixels. Valid padded rows, tail canaries
and invalid geometry/stride/size/format/null storage checks pass.

All2127 CTests pass across nine final-source Clang/GCC configurations,
including32-bit, optimized and sanitizer builds. Matching debug twins and30
loaded-application object audits pass. The10 scene audits permit only
cs_mac_scene/cs_surface_init imports; no generic fill or mutable global enters.
The native entry's changed-counter branch is source-reviewed and linked; it is
not claimed to be exercised by the hosted rendering suite.

The independently built64KiB original ROM twins match: SHA-256
`43c0296f808b85366fdbe55c9bcfd42e2daf9ef5fae77dfecfb85e5f80db0942`. Used ROM42736 bytes, workspace44496 bytes,
native data/BSS2168, conservative nonrecursive frame sum plus
assembly margin9492 within24320 reserved stack bytes,
largest frame604. The linked counter updater is present
and generic cs_surface_fill is absent. Seal, encoding, static layout, complete
link and corruption audits pass. This does not prove a full128K OS or hardware.

## Bounded worker use

One Claude call, five approved Reads, six model turns; no retry or worker test/
edit. Claude supplied102 C lines for the regression.
Codex corrected its unsupported stride64-only assumption, added valid padded
updates and explicit descriptor-corruption cases, and verified the frozen bytes.
Missing selected descriptor fields did not require another call. No Grok call
was needed for this focused change. Raw returned Read payloads are preserved;
archive-time live files are not asserted to be exact read-time snapshots.
Usage metadata makes no quota, billing or measured-savings claim.

All24 originals and existing private copies remain byte-identical. No extra
copies were made, UART bodies were not read/edited, and no HQ/raw research/
manual/credential/prohibited input was supplied to Claude. Ordinary Git/HQ/wiki
records only, with no OneNote/new agents/chats/automation.

## Authorized Snow rendering observation - passed

The owner explicitly answered "Yes—test the redraw ROM in Snow" (SRC-0196).
One fresh isolated Snow v1.5.0-317c188 Macintosh128K cold start used the exact
verified original ROM, with no Apple ROM, disk, saved machine state, shared
directory or serial bridge. The admitted executable hash still matches
32cf5285582c0f509e4f1ac38d86bfb90440bae9da43b7823bfb0841dea588e6.
Using an absolute workspace path avoided the previous launcher-path error.

Reset is logged22:57:02 UTC; the first snapshot at22:57:15.228 already shows
the complete APP/097, guest pattern and six phase marks. Counter0001 is visible
at22:58:28.985 after an automated left-button action started22:58:27.842.
A later right-button action starts23:03:20.726; its23:03:21.690 snapshot shows
further counter advancement. APP/pattern/phase marks remain intact in both.
These1.143/0.964-second host action/capture intervals include tool overhead;
they are not calibrated render times, a frame-by-frame flicker trace or a speed
ratio. Source review confirms the native loop no longer calls the full renderer
for ticks or counter changes. The earlier visible full-screen redraw is absent
from these post-input snapshots.

Some short automated clicks and a drag were not visibly credited. Diagnostic
UI interactions also occurred before the final increase, so later input counts
are not attributed one-to-one to injected actions. Exact delivery cause remains
unresolved: this qualifies the rendering change, not all-click input reliability
or human input. At our own linked diagnostic block0x1338, stage16, ticks3610,
presses1, trap count1, matching image seal4FC85F3D and fault0 were observed.
Only our original RAM was inspected; no disassembly, reference code or emulator
source. The trial closed normally at23:04:31 UTC; no unobserved continuity claim.
Original screenshots/profile/logs and returned observations are private.

## Published compiler and regression results - passed

Compiled source `2ea01129377d248ef48dd81d6f8873e948456ce6` passes both jobs in [CI run38820467440](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38820467440).
All2625 CTests pass: the2127-check complete matrix plus498 checks in two fresh
bootstrap profiles. The30 loaded-application object audits and inherited compiler
checks pass. Downloaded CI and local loaded-ROM images and full manifests match
exactly; the observed trial image has those same bytes. The previous own-ROM
profiles and every input fingerprint are unchanged; only stack-frame list order
is normalized for comparison, without dropping fields. CI runs no emulator.
This final evidence revision changes documentation only.

Both System1/ROM requirements and the
general runtime/desktop/native/physical/128K/parity/edition/B2/human gates remain.
[JSON](loaded-app-redraw-evidence.json); [previous actual boot](loaded-rom-app-evidence.md).
