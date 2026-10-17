# Drawing context claude prompt

Read only packet.txt once, 193lines, EOF known. Return usable original files personality/system1/quickdraw-view.h/.c, complete small replacement apps/native-drawing/workload.c and <=65line SPEC-0041 draft at docs/specifications/SPEC-0041-native-drawing-context.md. No test/build artifacts. Implement exact contract, bounded<=340 total lines, helper reuse and byte-identical old output. Return JSON {files:[{path,content}],handoff:{result,actionable_findings,tests_executed:[],blockers,decisions_needed}}. Core/header first, workload, then spec. No plan-only response.

Baseline `dd4be7bdbbf327ca0a05dd7243e496cc53c50dae`; [pinned contract](drawing-context-contract.md). Frozen public/original inputs only; implementation withheld from independent tester. Captures outside Git.
