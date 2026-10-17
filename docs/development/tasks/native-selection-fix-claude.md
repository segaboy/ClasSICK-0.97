# Claude native selection build-correction prompt

Windows11/PowerShell; baseline056165ecdfbb1eb6d079f5973e613aeab6f9afac.
Codex sole repository writer/final verifier. No tools,filesystem writes,web,
manuals,HQ,credentials,untracked originals,prohibited implementation,UART bodies,
new agents or tests. Return JSON proposed artifact only, tests_executed:[].

Inspect your original `personality/system1/quickdraw-select.c` and frozen
`quickdraw-select.h`, SPEC-0043, `quickdraw-view.h` and `quickdraw-rects.h`.
The exact API/contract remains unchanged.810 hosted CTests,10 width/endian audits
and8 defect controls passed; pinned GCC13 -mcpu=68000 O0 failed the import/data
audit. Error did not print symbol names. uintptr_t dynamic modulo is suspected,
not confirmed. Propose the smallest division-free alignment correction preserving
all behavior; do not weaken flags/audits,link helpers or change tests/contracts.
Return only the complete original core (<=230 lines,<=15 changed core lines),
findings explaining hypothesis versus verification,and<=100-word handoff.
Codex runs verification. Exact frozen hashes and initial packet are in the
[original brief](native-selection-claude.md); source above was supplied inline,
no implementation or failure logs were sent to the independent tester.
