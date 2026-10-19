# Native rectangle outline implementation -2026-10-19

SPEC-0048 / ADR-0054 / IMPL-0051 / TEST-0057; SRC-0287..0290.
Owner "OK, move on to the next task" accepts the proposed bounded native helper
implementation, independent tests, compiler/CI and ordinary Git/HQ records.
[Machine receipt](native-outline-evidence.json), [contract](../specifications/SPEC-0048-native-rectangle-outline.md).

The separate quickdraw-outline.h/.c implements the frozen API for both native
68000 and x86. It validates before rect access/effects, decodes big-endian signed
coordinates, computes the inward outline before clipping, sets black pixels and
increments paints exactly once with saturation. Zero pen, empty/inverted rect or
clip still counts as success. Native truthful/live/disjoint/synchronized caller
premises persist. No historical FrameRect equivalence,guest trap,pen-state ABI,
region,callback or global manager is introduced. Existing source/headers/layouts,
scenes,ROM/EFI/media inputs and prior candidates remain unchanged.

One Claude Opus5.5 low call produced104 core lines in22.2s with zero tools.
Independent Copilot Sonnet5.5 low supplied350 lines in126.72s from the frozen API,
contract and matrix,with one permitted prompt read and implementation withheld.
Both returned artifacts without retries,writes or tests. Copilot exceeded320-line
cap. Codex retained the inverse-map int64 oracle,corrected fixture call arities,
include path,8192-bit full-black count,dead offset loop and unsigned encoding.
Core behavior needed no correction; one misleading preflight comment clarified.
Raw streams remain outside Git. Provider counters are recorded,not billing or
cross-provider ChatGPT quota equivalence. All four account allowances unavailable.

Actual local checks: 847 CTests across three full hosted profiles (Clang64 Debug,Clang64 ASan/UBSan,
GCC32 Debug);838 oracle cases per profile,640 seeded,534 valid/304 refused,
128 painting cases and1705 assertions,zero failures. Complete initialized-object
snapshots include view/rect/canaries,padding and counter preservation. Eight
intentionally broken cores are rejected (solid fill,clearing,4-count,wrap,erases,
false clip edge,asymmetric pen mixup,zero-pen missing count).12 local compiler
objects pass no-import/no-mutable-data audit: Clang/GCC32/64 O0/O2,ARM LE/BE O2,
and matching native EFI-ABI object twins. Those twins are not linked EFI images.
CI native68000 O0/O2/Os twins and full existing profiles are pending below.

The first local configure lacked the bundled Python path; specifying it fixed
the harness. A generic GCC -m32 build selected an x64 resource compiler and failed
linking an existing resource object before tests; using the existing prefixed
i686 compiler selects the correct resource tool and the profile passes. Neither
failure was a helper defect. The mutation harness's erase anchor was corrected
before all8 completed. Failed attempts are not counted as passing tests.

Matrix dispositions:1-31,33-34 are executed oracle/state/layout cases;32 uses
existing regression suites plus immutable old source/header identities.35-36
include actual x86 width runs and compiler-only BE ARM/native68000,not target
endian execution;37 sanitizers passes;38 no-import/no-data audit;39 native68000
compiler-only CI pending;40 x86 and EFI-ABI objects,not new image integration;
41 named existing inputs/candidates preserved and normal CI regressions pending.
42 selected-view workload integration and43 legacy binding deferred;44 historical
equivalence not claimed,H1-H5 open. Do not summarize all44 rows as executed tests.

24 research originals/copies,UART,old records/releases/tags,7 published captures
and frozen wiki remain intact. No target/emulator/VM/reference/hardware run or
release. Source/edition/physical/full128K/B2/human/native-external-app/both System1
gates persist. OneNote/wiki permanently retired. Next proposed slice integrates
this helper into a new explicit paired native visual workload mode; boot trials
remain separately approved.
