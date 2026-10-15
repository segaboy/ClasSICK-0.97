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

## Actual boot and responsiveness still pending

The previous isolated boot proves the earlier candidate only. This candidate
requires fresh explicit owner go-ahead before Snow. Assess unchanged APP/097/
pattern/phases, kernel input and whether counter changes avoid visible full
redraws. No current elapsed-time, speed ratio, sustained-run or human-input claim
follows from host tests or static audits. CI results will be added after passing.
Both System1/ROM requirements and general runtime/desktop/native/physical/128K/
parity/edition/B2/human gates remain. [JSON](loaded-app-redraw-evidence.json);
[previous actual boot](loaded-rom-app-evidence.md).
