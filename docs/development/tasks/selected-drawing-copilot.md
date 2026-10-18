# copilot selected drawing prompt -2026-10-18

You are a supervised external worker for ClasSICK0.97 on Windows11/PowerShell.
Baseline c9aa8494a5ce4aa7ef0febbb55e0f945d20be5fb plus frozen SPEC0046/header below.
Codex is sole repository writer/integrator/verifier. Read only this packet through
EOF; no discovery,commands,edits,tests,web,MCP,delegation or other paths. Inputs
are reviewed public contracts and original project code only. No private HQ,
credentials,untracked research originals,manuals,Apple/Microsoft implementation,
ROM/media/binaries/disassembly,emulator source,manual samples or UART bodies.
Return original GPL3 C11 artifacts with strict conversions,freestanding68000/x86,
bounded storage,no allocation/libc/mutable globals. No target trial/release.
Return JSON {replacements:[{path,old,new}],files:[{path,content}],actionable_findings:[],
tests_executed:[],blockers:[],decisions_needed:[],handoff:"under180words"}.
Keep total proposed C <=400lines. Tests are proposals until lead executes them.

Distinct risk deliverable: tests/code-select-drawing-independent.c complete
independent oracle artifact only, <=350lines. New implementation/logs withheld.
Reuse old fixture constructor/helpers supplied below; do not copy its entire
old suite. Use real PREPARE/LOAD original CODE0/1,below512,above40,table8; code
<=100bytes with staging128. Raw push-immediate2F3C,store-immediate23FC,RTS4E75,
MOVEA A5,A0=204D,pushimmediatepredecA0=213C,pushA0=2F08 supported. Scene unchanged.
Check actual per-pixel expected masks independently,not native helper as oracle.
Fresh current write must chooseotherport,distinct offsets/clips. Verify exact
stop frame and scratch on refusal versus completedprefix,callerunchanged,oneuse
andretirement. Keep original unresolved storage live iftestingnonquiescence.
Avoid assumption count5 for4 services,invalidating valid alternativeg or vacuous
assertions. You may generate multiple fixtures withinbounded total350lines.

Exact reviewed inputs:
- `docs/specifications/SPEC-0046-selected-port-drawing.md`: `f280f9eab1133b28d452d6da83a0eb76f709c9bd019a8d52384911fcb9725d43`
- `docs/specifications/SPEC-0045-integrated-port-selection.md`: `b4886c24e72bf4462b0dafd486401e1b39cdc5faa4c4f9d21ab2a3135ee408ad`
- `personality/system1/code-select.h`: `e798c5c84ab896c279566b3b7465b7e039c7344f9f0bcfaa8956686d46921890`
- `personality/system1/code-image.h`: `6d95d79e9b33817ce2333c4d1323e92275feab884f53d3f10b439606ea91b50d`
- `personality/system1/code-run.h`: `1c4489624e1abccd9dc654aa51d962210c6a10ce37ccbd84ac1447037ff8238b`
- `personality/system1/quickdraw-select.h`: `cd72e096f6a9365ede45fd859df755fec021eb1986002d5ca8269a6c8f838624`
- `personality/system1/quickdraw-view.h`: `4a93567872049f5142a3980eba501c7a2e027c15de794dde11fbbd93cc3bbee7`
- `personality/system1/quickdraw-rects.h`: `db9facdb5ef3742c3c8935255e3f627c30d31b323a8223e470d9851a48c81e09`
- `core/memory/guest.h`: `5a89e549470bd0f3ffe50e7fb714342ff7dc6af0d49a0c154a7baebdf7e113ff`
- `core/services/traps.h`: `a3346547364a2237027dbe1d134b81129e80f9731f4c49c813fc9f68563fd782`
- `apps/loaded-rom-app/probe.h`: `5800d08070c5e88b55a634bf9559c7faa6ab19dbcbde749c8650a4eba03f5f5c`

Frozen API:
```json
{
  "personality/system1/code-select.h": "e798c5c84ab896c279566b3b7465b7e039c7344f9f0bcfaa8956686d46921890",
  "docs/specifications/SPEC-0046-selected-port-drawing.md": "f280f9eab1133b28d452d6da83a0eb76f709c9bd019a8d52384911fcb9725d43"
}
```
Complete packet/captures outside Git. CLI permissions are not a hard filesystem sandbox.

One bounded correction prompt (initial call exhausted its per-call budget):

BOUNDED CORRECTION: The previous call reached its per-call credits before any
artifact. Return only120..180lines for tests/code-select-drawing-independent.c,
reusing old helpers by #define main cs_old_selection_main then
#include "code-select-independent.c" then #undef main. All supplied helpers and
old-suite symbols are then available; do not reimplement them. New main runs only
the new drawing cases. Keep new implementation/logs withheld. Prioritize actual
pixel routing after direct current writes,offset/clip,paint+erase,NIL/odd/badspan
refusal,all4patchedslots,PRF1preservation,priorpixelsretained,cleanup/oneuse.
Use old make(),raw(),push(),store(),rd(),retire(),frame_eq()/zero() as needed;
new go uses explicit cs_image_run_selection_draw. Reference original main symbol
if necessary to suppress unused warnings. No need reproduce old helper source.
Read ONLY prompt.txt,which already contains complete inputs. Never open settings
or any other file. Return requested JSON with original code immediately after
reading packet; no catalog/planning/prose-only result. tests_executed=[].
END OF CORRECTION
