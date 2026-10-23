# Claude ordered capture implementation assignment

2026-10-22;SRC-0367;one bounded primary,Windows11.
Actual author prompt followed at dispatch by the frozen SPEC-0063/header and
complete public type-header graph. Raw packet/captures stay outside Git.

```text
Implement ONLY apps/window-scene/capture.c for ClasSICK0.97 on Windows11. Goal:paired-native-preview-2,original native68000 Mac and native x86-64 PC512x342 window parity. Current task is an isolated sequential ordered capture/drain helper;no hardware/boot binding,MMIO,ISR,VM/emulator,graphics,fixtures or historical claims. Codex is sole repository writer/integrator/verifier. You return captured proposed code only.

Use ONLY supplied frozen original contract/header and reviewed public type headers. No tools,commands,paths,edits,web,MCP,delegation or tests. All dependencies/types are below;system stdint/stddef headers are ordinary compiler interfaces. Produce <=350C lines,strict C11,Werror/Wconversion/Wsign-conversion,freestanding no allocation/globals/mutable statics/CRT. Only two allowed imports:cs_window_input_sample and cs_window_input_cancel. Avoid implicit memset/memcpy helpers;initialize metadata/last field-by-field,unused records/padding are unspecified. Validate scalar state before owner calls. Producer APIs must NEVER dereference owner. Foreground ownerOK0/CHANGED1 are success;all other values fault. Capture descriptor<=256,per-native-frame<=512. No motion coalescing. First-fault stage/cause survive stop until explicit restart. Stale epoch/refusal no effects. Follow exact phase/results/ordering fromSPEC-0063,not the historical draft. begin does not read old capture. Only fresh lifetime begin may reset epoch1;restart cannot wrap. Caller exclusion/lifetime/disjoint graph preconditions are explicit;not hostile-pointer validation or interrupt safety.

Return JSON {"files":{"apps/window-scene/capture.c":"complete source"},"handoff":{"result":...,"actionable_findings":...,"tests_executed":[],"blockers":...,"decisions_needed":...}}. Handoff<=100words. No tests claimed. Use GPL-3.0-or-later and Copyright(C)2026 Dean Howell. EOF supplies all input. Public base revision:f46d5bbea9083ff18898192c69f82a8dc6202d50;new header/spec are frozen lead-original inputs,not part of that commit.

```

Input22,167B/439lines,SHA256`585aa6bd69cc990e90e7081fea2cabdca869c198a0a70d3468b7061e216cb803`.
One53.97s call,259 lines within350 cap,zero tools/writes/tests. Read-only tools,
dontAsk,no MCP/memory/hooks/slash commands/session persistence,max2turns,420s
outer bound,Opus5.5 low. No manuals/HQ/original drafts/implementation bodies.
Lead stop-path/static-assert fixes and STATE clarification recorded separately.
No second worker,retry or qualification;initial lead tests were API-only;later CI raw-snapshot correction is explicit.
