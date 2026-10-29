# Original PC286 span oracle assignment

Completed bounded qualification,2026-10-29;baseline8a197ffe32853bd4c346aa2d0a833dfee7b6a903.
One fresh Copilot Sonnet5.5 Low call;Windows11/PowerShell. Frozen public-only
packet SHA2562b9548b67f4443496db4d0e5b98b5083fc32614b8b981f096a55a314d2012098.
Inputs:arch/i286/c16.h;probes/pc286/{bank-plan,bank-catalog,store4-model}.h;
reviewed proposed span-build.h/contract and lead-frozen literal expectations.
The author implementation/tests were absent. The private frozen packet/capture
is the exact dispatch record;this file is a public scope index,not a new dispatch.

Deliver one original strictC99 tests/pc286-span-build-oracle.c with always-on
checks,nonzero failure exit,host1MiB arena/snapshot and literal boundary results.
Cover null-tuple refusal,segment0/nonzero offset,65535 split,exact64KiB end,
last1MiB write,95 reachable chunks,representation/capacity precedence,late
invalid chunks,arguments,cross-bank BE store and refusal preservation. Compare
only initialized copies of identical objects;do not depend on struct padding.
Bind validates representation only,not physical ownership. No target dereference.
Return JSON source and <=150word handoff;artifact<=270lines,tests_executed=[].
View frozen input once through EOF;no other paths,commands,edits,web,MCP,
delegation or tests. Exclude HQ,manuals,original research,credentials,held bodies,
Mac/CPU/emulator/Apple/Microsoft implementation and all other prohibited inputs.

Result/disposition and actual execution:[canonical receipt](../pc286-span-build-evidence.json).
