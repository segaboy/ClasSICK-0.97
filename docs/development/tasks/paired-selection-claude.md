# claude paired selection workload prompt -2026-10-18

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

Primary artifact: apps/selected-drawing/workload.c <=180lines implementing
the frozen API. Also exact minimal #ifdef CLASSICK_SELECTED_DRAWING replacements
to platform/mac128k/native-drawing-entry.c and apps/boot-scene/scene.c. Preserve
old branch/source behavior. In Mac loaded_app use caller-owned selected workspace;
choose proper pixels/result fields and run selected function. PC boot_workspace
uses selected workspace/result in explicit mode; service probe stillruns,prepare
allocates it from arena and publishes its composite,result.draw diagnostics.
Keep cs_scene_render legacy,with its existing local1032-byte port. No source
files elsewhere or build-script proposals needed. Total C artifact delta<=300lines.
Use root-local selection/views/plan; avoid partial array aggregate zeroing that
can emit memset on68000. View validity comes from original cores,not reimplemented.

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
- `apps/native-drawing/workload.c`: `18c5d193e78d6a476104d28fcac3705977485cb47b2f5bc9de3937500b4af1f5`
- `platform/mac128k/native-drawing-entry.c`: `512ca711319f6aea920bad165757bd231213d4815849e4ec24d673e675a6d737`
- `apps/boot-scene/scene.c`: `e0e9fb5b74e4327672927b7a5af3f3b88f1b43d124feea6aa277e3704d4e6800`

Complete packet/captures outside Git; CLI controls are not a hard filesystem sandbox.
