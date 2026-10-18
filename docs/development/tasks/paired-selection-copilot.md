# copilot paired selection workload prompt -2026-10-18

You are a supervised ClasSICK0.97 worker on Windows11/PowerShell. Public baseline
71ccf7af12f2226e798d531cc3b643afa81ed2ad plus frozen SPEC0047/header below.
Codex alone writes/integrates/verifies repos. Read only this complete packet to
EOF; no commands,edits,tests,web,MCP,delegation,discovery or other paths. No HQ,
credentials,untracked research originals,manuals,Apple/Microsoft code/media/ROM,
disassembly,emulator source,manual samples or UART implementation/test bodies.
Original GPL3 C11,strict conversions,freestanding68000/x86,no libc/allocator/
mutable core globals. Return JSON {files:[{path,content}],replacements:[{path,old,new}],
actionable_findings:[],tests_executed:[],blockers:[],decisions_needed:[],handoff:"<180words"}.
Output usable artifacts immediately; tests_executed must be[]. No target run/release.

Distinct independent oracle: tests/selected-drawing-independent.c <=180lines,
new implementation/logs withheld. Focus real per-pixel twoport predicates+union,
FNV/black counts,phases/current/selections/counters,poison/reset/repeat/null/canaries.
Use original cs_product_scene and cs_boot_scene_prepare compiled with
CLASSICK_SELECTED_DRAWING for Mac mono512x342 vs PC RGBA640x480 centered parity.
Independent expected pixels must not call the native view/draw helper. You can
keep the scene comparison to a compact loop checking monochrome/RGBA equality,
and show input update preserves every pixel outside the translated80x20counter.
Use heap/calloc for large frame arrays,free after; no giant stack arrays. Test
only hosted functions,not apps/emulator. Include whole deterministic source.
Do not produce a prose-only plan; artifact<=180lines fits per-call budget. Only
prompt.txt exists; do not read any settings. Use supplied APIs,not imaginedfields.

Frozen interface:
```json
{
  "apps/selected-drawing/workload.h": "98063deca9234f02febcc71da6a47d9fcf64781f6acb3686f9b8d879c00c9664",
  "docs/specifications/SPEC-0047-paired-selected-drawing.md": "ffb25d959be3a256335104bf33d499a4642eaa065919437a5453128bc95233dc"
}
```

Exact reviewed inputs:
- `docs/specifications/SPEC-0047-paired-selected-drawing.md`: `ffb25d959be3a256335104bf33d499a4642eaa065919437a5453128bc95233dc`
- `apps/selected-drawing/workload.h`: `98063deca9234f02febcc71da6a47d9fcf64781f6acb3686f9b8d879c00c9664`
- `apps/native-drawing/workload.h`: `5c536aa74ac4799a941b68d6246f42a68937d77aa37f18ff3c73d79560872194`
- `personality/system1/quickdraw-select.h`: `cd72e096f6a9365ede45fd859df755fec021eb1986002d5ca8269a6c8f838624`
- `personality/system1/quickdraw-view.h`: `4a93567872049f5142a3980eba501c7a2e027c15de794dde11fbbd93cc3bbee7`
- `personality/system1/quickdraw-rects.h`: `db9facdb5ef3742c3c8935255e3f627c30d31b323a8223e470d9851a48c81e09`
- `apps/product-scene/scene.h`: `7d5b50af59be3e25a67837d7183d0fa50afd36b9f0b3aaf1a1fb1ddea772ccf0`
- `apps/boot-scene/scene.h`: `5642a9ec3d8ebf31579a61d0a5b0e6567d2db8756d429f041c66753f3d00b9ef`
- `core/memory/arena.h`: `e26f9a33e8e1bc534ff738c131a2f3317159de600c5a59fa34f6788bbab8ceed`
- `platform/pc/framebuffer.h`: `3abdaf4d591fc79135dc6e4021dda496855a9efca1272725f8370a6517ca68d9`
- `core/graphics/surface.h`: `31de605b79282fac7f4208e28aab328662df975a03028ac16fe179672d6dc264`

Complete packet/captures outside Git; CLI controls are not a hard filesystem sandbox.
