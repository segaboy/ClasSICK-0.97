# Loaded application redraw regression assignment - 2026-10-15

Owner "OK, next step." (SRC-0193) accepts the proposed focused responsiveness
fix within SPEC-0035 / IMPL-0039 / TEST-0043. Codex remains sole writer and
implements it. One supervised Claude artifact call under SRC-0149; Grok is
reserved for a concrete unresolved gap, not a second routine review. No new
historical contract, QuickDraw/guest ABI, loader or CPU changes. A future Snow
trial still requires fresh owner go-ahead. Preserve24 research originals and
their existing private copies; no duplication or modification needed.

## Artifact requested

Return one original C function `static void redraw(void)` (plus small static
helpers if useful), at most180 C lines, for tests/loaded-rom-app.c. Do not edit
files or execute anything. Use the existing REQUIRE macro and included headers.
The lead will integrate/register it as `loadedapp.redraw` and run all checks.

Implementation contract: existing cs_loaded_app_scene(s,result,presses) retains
its complete-frame output and descriptor validation. Its overlay uses original
bounded byte fills after validation instead of generic per-pixel fills. New
`int cs_loaded_app_scene_input(const cs_surface *s,uint32_t presses)` validates
a512x342 MONO1_MSB surface with cs_surface_init before writes. It clears and
redraws only logical x[352,432), y[286,306), including zero pixels. It renders
the same four original decimal glyphs as the existing scene, low four digits
(modulo10000). Every other logical pixel and all row padding/tail bytes remain
unchanged. No result pointer or retained state is accepted by this updater.
Invalid descriptors/null are refused without changing storage. Inputs assume
ordinary live/disjoint native descriptor/storage objects, as existing graphics.

The native loop still polls and records ticks/presses. It invokes only the
counter updater when presses changes, avoiding invisible-tick full redraws.
This loop binding is lead-reviewed, not something your artifact executes.

## Frozen output evidence

Before any implementation change, a separately compiled harness used the
published old renderer at1b455ea and generated these original frame fingerprints.
FNV1a32: start2166136261, for each of21888 bytes: (h XOR byte)*16777619 with
uint32_t wrap. Surface512x342, stride64, buffer initialized0xCC. Each row below
is {value,pattern,phase,success,presses,expected_hash}; other result fields zero.

```c
{97,0x5A5A5A5A,6,1,0,3049100689u},
{97,0x5A5A5A5A,6,1,1,144985001u},
{97,0x5A5A5A5A,6,1,2,2718142593u},
{97,0x5A5A5A5A,6,1,9999,1255755017u},
{97,0x5A5A5A5A,6,1,10000,3049100689u},
{97,0x5A5A5A5A,6,1,UINT32_MAX,3866680433u},
{0,0,0,0,0,705968457u},
{97,0x5A5A5A5A,4,0,0,182201649u},
{97,UINT32_MAX,5,0,7,3247734721u},
{3610,0x80000001,6,1,17,666794281u},
{UINT32_MAX,0,6,1,10001,3122324361u},
{97,0x01010101,2,0,8,2519077233u}
```

Check these full frames; then compare incremental counter sequences with full
frames, including9999->10000, UINT32_MAX and repeated values. Exercise padded
rows and tail canaries, rejection without partial writes, and arbitrary original
non-counter pixels preserved byte-for-byte. No timing thresholds or benchmark
claims, no test that just repeats the byte-fill algorithm. Ordinary original
project fixtures only, no manual samples or historical/reference pixels.

## Read boundary and gates

Read this brief once (offset1 limit120), tests/loaded-rom-app.c offset1 limit10
and offset133 limit40, apps/loaded-rom-app/scene.h offset1 limit25, and
apps/loaded-rom-app/probe.h offset1 limit40 only if needed. Maximum five Reads;
no broad scans, additional tools, file edits, commands, web, MCP, agents or
retries. Stop with a concise question if a necessary contract is absent.
No HQ/raw research/manual/credential/Apple/Microsoft code, ROM, binary, asset,
disk image, disassembly, emulator source or UART body. Native-app/physical/
128K/parity/edition/B2/human and both System1/ROM gates remain. No chats,
automation or OneNote. Research is not implementation authority.
