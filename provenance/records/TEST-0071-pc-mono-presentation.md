# TEST-0071 - PC monochrome whole-target and partial-publication oracle

2026-10-22;SPEC-0060;IMPL-0062;SRC-0351/0352.
Claude original431-line API-only oracle;lead adds alpha and fixed-object numeric
wrap checks. Freeze precedes implementation;test input excludes new bridge code.
Real existing framebuffer/surface dependencies;separate renamed-presenter object
injects refusal after0,1,middle,last completed rows and asserts arguments/progress.
Pixel oracle checks full physical byte arrays,not only changed pixels. Seeded
authored asymmetric Mono patterns,corners,RGB/BGR,exact/odd/even/max-width target,
pitch/tail,initial/update/reuse,readonly sharing and aliases/refusals are exercised.
Black/white output cannot distinguish RGB vsBGR channel ordering;both accepted
formats are tested,not colored-channel conversion newly qualified.

Per Clang64/sanitized/GCC32 run:95 normal cases/834 checks/49 complete frames;
87 fault cases/112218 checks/9 complete and32 partial frame comparisons. Three
local compiled defects reject. Full final Clang64 regression320 plus3/3 focused
sanitized/GCC32 CTests pass. Ten compiler import audits are separate checks.
These are E1 original RAM targets,not device/GUI/human/historical evidence.
TEST-0070 session/boot cases remain unexecuted;no new fixture file or target trial.
[Profiles/source pins/CI](../../docs/development/pc-mono-present-evidence.json).
