# PC window session worker prompt

2026-10-22;SRC-0354;baseline `b46876d18f0ed70eb62df256445cd310bb1c8a06`.
One Claude primary,public-only input folder;no OS isolation claim. Exact header/spec
identities in the [receipt](../pc-window-session-evidence.json). No implementation
from excluded sources or untracked originals.

```text
ClasSICK0.97,Windows11,supervised Claude primary. Codex sole repo writer/integrator.
Implement ONLY platform/pc/window-session.c plus tests/window-session.c from the
lead-frozen original API/SPEC-0061 below, using existing public APIs. Return one
JSON object {files:{path:complete_source,...},handoff:{result,actionable_findings,
tests_executed:[],blockers,decisions_needed}}. Handoff<=180words. No tools/reads,
commands/edits/web/MCP/subagents/manuals/HQ/untracked originals/UART bodies.
All inputs inline through EOF. No tests actually executed by you. No UEFI or
boot-binding,release,target-trial or historical claims. No header/spec changes.
Max600 combined source lines,<=280 production. GPL-3.0-or-later/Copyright(C)2026
Dean Howell. Strict C11 GCC/Clang -Wall -Wextra -Wpedantic -Werror -Wconversion
-Wsign-conversion,ILP32/LP64 clean. Production freestanding,no libc/memcpy/memset,
allocation,globals,hidden dependencies or double-width division. Scalar zero/copy.
Initialize fields before reading; never copy a live bound self-referential graph.
Use CS_WINDOW_INPUT_OK/CHANGED both accepted. Begin must obtain cursor token via
bind, then owner_begin then cancel, one publication, then bus_begin. First-poll
NONE delivery must not leak old data; terminal polls and repeated stop inert.
Freshness never inferred from counters. Bad phase should return STATE inert.

Tests are ONE standalone C main, normal linked real session+all real dependencies;
fault build compiles SESSION OBJECT ONLY with selected imports renamed:
cs_ps2_bus_poll_delivery=cs_test_bus_poll,
cs_input_pop=cs_test_input_pop,cs_pc_window_packet=cs_test_packet,
cs_window_input_cancel=cs_test_cancel,cs_pc_mono_present=cs_test_present,
cs_window_input_begin=cs_test_begin,cs_ps2_bus_begin=cs_test_bus_begin.
Test source compiled CS_PWS_FAULTS in fault build,wrappers delegate real APIs
unless explicitly injecting;regular build defines no wrappers. These are test
seams only,not API additions. Do not globally rename linked dependencies.
Normal test can use mock port callbacks/no device to test idle/start/clock/stop;
fault-mode bus wrapper can emit specified raw packets/loss while leaving real
owner,renderer,cursor and presenter active. No real I/O. Assert exact publication
and owner counts/order,start suppression,initial down ignored until real release,
title drag/clamp and body pattern,identical fresh reports even saturated totals,
NONE no publication,Space/Escape draining,max8,queue/controller/clock failures
discard mouse receipt/no new cancel/publish,owner fault,partial publication,
begin/cancel/first-publication failures before bus begin,terminal inert stop.
Preflight check null,capacity,aligned numeric wrap,full-span aliases with no caller
mutation;use enough live storage,never dereference fabricated numeric addresses.
Compare snapshots/guards and fields meaningfully;don't mirror production algorithm.
Mocks must assert received inputs/order not simply force successes. Output cases/
checks/frames actually reached;fail clearly. Normal/fault modes both must compile.
Do not generate CMake/docs or whole unchanged files. Return artifacts only JSON.

```

420.09s cap,complete production/partial tests salvaged;no retry. No worker tools,
writes or executed tests. Lead corrections and actual checks in the receipt.
