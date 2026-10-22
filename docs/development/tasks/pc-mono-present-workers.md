# PC monochrome presentation worker prompts

2026-10-22;SRC-0350/0351;baseline `63f6b2bbac99900db48f8ec85ffc8cc49e1f5806`.
One Grok primary and distinct-risk Claude API-only oracle,in parallel;no new
implementation sent to Claude. Frozen uncommitted lead API/spec plus exact public
dependencies identified in the [receipt](../pc-mono-present-evidence.json).
Input-only folders and captures stay outside Git;controls are not an OS sandbox.

## grok

```text
ClasSICK 0.97 supervised worker,Windows11. Codex is sole repository writer/integrator.
Current release goal:paired-native-preview-2,native Mac68000/x86 PC draggable window;
this isolated PC presentation bridge advances it,no boot/session binding or trial.
Use ONLY frozen original public inputs below through EOF. No tools,reads,commands,
edits,web,MCP,subagents,manuals,HQ,untracked originals,UART bodies or third-party
implementation. Return usable original artifact JSON and handoff under180 words
(result,actionable_findings,tests_executed=[],blockers,decisions_needed).
Do not run tests or claim proposed tests passed. All existing gates persist.
Code must be GPL-3.0-or-later/Copyright(C)2026 Dean Howell,C11,strict GCC/Clang
-Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion,32/64-bit clean.
Primary:implement ONLY platform/pc/mono-present.c from the frozen API/spec.
Return JSON {files:{"platform/pc/mono-present.c":"complete source"},handoff:{...}}.
Max240 source lines. No tests,CMake,docs or header changes. Stateless,no globals,
allocation,libc or hidden helpers. Imports allowed only cs_fb_init,cs_surface_init,
cs_fb_present. Use scalar copies/stores (no compiler memcpy helper atO0).
Reject full spans/aliases before pixel reads or caller writes. Readonly spans may
share. Validate source/target through existing APIs into locals. No output writes
on preflight error. Use one row;store all RGBA bytes incl alpha;no device reads or
direct writes. For initial iterate physical rows;update only342 logical rows.
Propagate failed dependency cause and exact successful-row progress;no further
row calls after failure. No overflow/pointer UB on rejected numeric ranges.

```

## claude

```text
ClasSICK 0.97 supervised worker,Windows11. Codex is sole repository writer/integrator.
Current release goal:paired-native-preview-2,native Mac68000/x86 PC draggable window;
this isolated PC presentation bridge advances it,no boot/session binding or trial.
Use ONLY frozen original public inputs below through EOF. No tools,reads,commands,
edits,web,MCP,subagents,manuals,HQ,untracked originals,UART bodies or third-party
implementation. Return usable original artifact JSON and handoff under180 words
(result,actionable_findings,tests_executed=[],blockers,decisions_needed).
Do not run tests or claim proposed tests passed. All existing gates persist.
Code must be GPL-3.0-or-later/Copyright(C)2026 Dean Howell,C11,strict GCC/Clang
-Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion,32/64-bit clean.
Distinct-risk independent TEST-0071 artifact. You do NOT receive the new
bridge implementation. Return JSON {files:{"tests/mono-present.c":"complete source"},
handoff:{...}}. Max450 source lines. One test source with regular main;compile it
twice with or without CS_MONO_FAULTS. Normal build links real bridge+framebuffer+
surface. Fault build compiles BRIDGE ONLY with cs_fb_present=cs_test_fb_present;
test defines that wrapper and can delegate successful rows to the real presenter.
Do not rename dependencies globally. Fault mode should verify initial/update row
coordinates/width,call bounds,progress and early failure after k successful rows.
Normal mode:independent full target-byte oracle for asymmetrical Mono MSB patterns,
odd/even/exact/large geometries,RGB/BGR,pitch padding/tail,initial white margins,
update-only canvas writes and source/workspace guards. Avoid ~100million CHECK
increments;compare buffers and report mismatches precisely. Include max width8192
with modest height,not an8192-square allocation. Snapshot report,row/device bytes
on preflight refusals;test NULL/mode/source/target/capacity/wrap and representative
full-span aliases with all other inputs valid. No invalid-pointer dereferences.
Cases with overlapping valid native objects must allocate enough live/aligned
storage. Test report16B. Use CHECK returning failure clearly;count actual cases/
assertions/frames in output. Source and readonly descriptor bytes stay unchanged.
Tests must compile/run directly with only these dependencies,no test framework.
No production hooks or implementation guesses;only the frozen contract is authority.

```

Grok cap240.97s,no usable code;no retry. Claude231.05s,431 lines,zero tools/writes/tests;JSON packaging corrected. Codex original production fallback and two test additions;[actual results](../pc-mono-present-evidence.md).
