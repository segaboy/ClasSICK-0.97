# PC monochrome presentation evidence

2026-10-22;SPEC-0060;ADR-0069;IMPL-0062;TEST-0071;SRC-0349..0352.

Implemented the isolated stateless PC presentation bridge. It borrows one logical
512x342 Mono1 RAM canvas and one RGBA row (4W<=32768 bytes),reuses the unchanged
framebuffer APIs,and centers unscaled. First publication initializes white
margins;updates touch only the logical canvas. Source/descriptors,pitch padding,
target tail and unused workspace survive. Full-span preflight rejects aliases,
wrap and invalid objects before caller effects;progress reports retain completed
rows on an injected presenter refusal. Device access remains in the existing
presenter;source review finds no bridge framebuffer reads/direct device writes.
RAM tests do not measure physical device reads,scanout atomicity or MMIO recovery.

Actual local results:
- 320 final Clang64 regression CTests;3 sanitized and3 GCC32 focused checks (326).
- Per profile:95 normal cases/834 assertions/49 complete frames,and87 fault cases/
112218 assertions/9 complete frames plus32 partial-publication comparisons.
- Three locally compiled defective variants rejected:inverted black,wrong odd-size
centering and false completed-row progress. Compiler success alone is not a pass.
- Ten isolated object/import audits:Clang/GCC32/64 O0/O2 and Clang ARM little/big
endian. Only cs_fb_init,cs_surface_init,cs_fb_present imports;no mutable globals.
Compiler .su diagnostics are retained in the receipt;not a whole-kernel stack,
ROM or128K fit certification. No new native image or hardware result.

Grok primary reached240.97s cap with109 bytes of an unfinished license prefix,
no usable C or adopted lines. No complete usage/resolved-model record;timeout
does not prove zero spend or backend cancellation. No retry or third worker.
Codex wrote the83-line bridge from the frozen API/contract. Claude Opus5.5 low
independent oracle completed231.05s,431 source lines from11740 frozen public
bytes,zero tools/writes/tests;new implementation was withheld. Artifact had a
preface/fence and an extra outer JSON brace,removed during capture parsing.
Lead added alpha and fixed-object numeric-wrap checks;no worker fixture expectation
was weakened. Worker handoff's "compile-must-fail" wording is corrected:three
compiled behaviorally defective controls were actually executed and rejected.
Provider reports$0.5967904 list/session cost,not a balance or Codex quota saving.
Claude account-window snapshot is private;other balances unavailable. No quotas
or credentials enter public Git. Copilot/Argon idle;qualifications reused.

Frozen packets/captures/commands/results remain outside Git at
C:/ClasSICK/mono-present-20261022-a. [Input identities/actual receipts](pc-mono-present-evidence.json),
[worker prompts](tasks/pc-mono-present-workers.md),
[native interface](../../platform/pc/mono-present.h).

Only the bridge and test/compiler/CI integration changed. Existing framebuffer,
scene,cursor,controller,UEFI/ROM source lists,UART,released assets/tags,24 originals,
held brief,old rows/checkpoints and retired wiki are preserved. No window session,
boot binding,VM/emulator/reference/hardware trial or release. TEST-0070 full binding
cases remain unexecuted;this leaf tests presentation portions only. Mac M1-M7,
PC GUI/human/CPUID,source/role/historical/edition/physical/full128K/B2/native-app/
Linux/mini-vMac/both System1 directions persist. Next:neutral PC window session
using explicit packet/loss receipts,then the opt-in UEFI binding and trace.
