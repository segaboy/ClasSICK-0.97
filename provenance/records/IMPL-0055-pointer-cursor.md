# IMPL-0055 / TEST-0063 - Shared native pointer and cursor helpers

2026-10-20;SRC-0319..0322;SPEC-0051/0052;ADR-0059/0060.
Original GPL-3.0-or-later portable C for native68000/x86,not historical behavior.

New pointer core validates complete samples,clamps without signed overflow and
publishes one coherent movement/edge/cancel transition. New cursor core uses
caller-owned state,art and bounded separate payload;validates before writes,
clips the512x342 canvas,restores exact Mono/RGBA backgrounds and defines explicit
retirement/revision/repair transitions. Existing scene,driver,ROM/EFI source
lists,legacy APIs and released assets are unchanged. No new artwork or binding.

Claude drafted four source/header files. Copilot's combined test request returned
no artifact;one reduced request supplied an independent cursor per-pixel oracle,
with implementation withheld. Codex supplied the smaller pointer reference test,
corrected fixture lifetime/rectangle errors,integrated bounded scratch/index
refinements and wired verification. No manual,HQ,untracked original or UART input
went to workers;no worker wrote the checkout or ran tests.

Actual local verification:874 CTests across Clang64 debug,ASan/UBSan and GCC32,
zero failures;24 width/endian/EFI-ABI object audits. Pointer direct test uses3000
reference transitions/13675 assertions. Cursor oracle compares complete storage
on10 layouts;its cases cover corners/hotspots,byte residues,transparency,moves,
background restore,guards and lifecycle/refusals. Theoretical48/56B state layout
is now checked by native compile-time assertions. Native68000 CI now passes30 object compilations and6 matching-twin closure audits,with no imports or mutable data;actual48B state layout passes. No target ran. The receipt pins the exact verified source and CI result.

TEST-0061/0062 remain original proposed inventories,not blanket execution claims.
Quarantine/retire-while-live is tested;arbitrary expired/unmapped-pointer recovery
is not qualified. Explicit scratch and actual compiler frames are reported
separately;the96B local-workspace target is not a96B physical stack-frame promise.
No full128K,whole-ROM budget,hardware input,window,GUI/human or boot qualification.

[Exact inputs,worker accounting,dispositions and actual results](../../docs/development/pointer-cursor-evidence.json).
Mac M1-M7,PC GUI/human input and existing source/role/historical/edition/physical/
B2/native-app/Linux/mini-vMac/both System1-ROM gates persist. No target trial or
release;24 originals,held brief,UART,old records/assets/tags and retired wiki kept.
