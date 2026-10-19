# IMPL-0053 - Opt-in continuous native PC preview

2026-10-19; SPEC-0050 / ADR-0056 / TEST-0059; SRC-0298..0300.
Claude supplies four exact keyboard-loop replacements,104 proposed test lines and
a60-line specification; Codex adopts them with strict counter-area comparisons,
real outline-renderer wiring and a legacy-linked rejection control. The native
outline build alone sets CLASSICK_PC_PREVIEW_SESSION=1 on keyboard.c. Its normal
deadline and Escape exit are suppressed; startup/argument/fault checks persist.
The default flag0,other native modes,public ABI,UART,PS2 decoder/controller,scene,
drawing core and Mac inputs are unchanged. Profile identity is included in EFI
and media receipts. [Evidence](../../docs/development/preview-session-evidence.md).
