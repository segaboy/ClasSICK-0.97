# Focused guard bypass check - 2026-10-15

Owner SRC-0183 authorizes SPEC-0034 X-1 implementation only. Codex is the sole
repository writer; Claude supplied tests. Check one risk: can a successful
guarded step publish CPU/RAM/gateway effects before all required checks, or can
an actual access/service path bypass its guard? This is not a general review.

Only Read these original public files in C:/Repos/ClasSICK-0.97:
- this brief (under40 lines);
- core/cpu/m68k-guard.h (40 lines);
- core/cpu/m68k-bus.c (390 lines, chunks1..120,121..240,241..360,361..390);
- tests/m68k-guard.c lines139..190 if useful (atomicity/control tests).
Each Read <=120 lines. No EOF probes. At most7 Reads. No other files, commands,
edits, tests, web, MCP, extra agents/chats, automation or OneNote. No HQ, manuals,
untracked originals, Apple/Microsoft code/ROM/binaries/resources/disk images/
disassembly/emulator source, manual samples or private heap reconstruction.
Standing scoped public/original-code transfer SRC-0149 applies.

The native policy is trusted, pure, immutable, synchronized and non-reentrant;
live/aligned/truthfully sized native inputs are preconditions. It is not a
sandbox for a malicious native hook. No session or owned-image fetch authority:
ordinary map EXEC remains required, all A-lines denied, strict24 only. Existing
unguarded entry points intentionally retain their legacy semantics and do not
claim guard protection. Report diagnostics can change on ordinary refusal;
native-envelope refusal must preserve outputs/report. Distinct backing aliases
and control/context/output overlaps refuse conservatively. Referenced hidden
native objects are not discovered by traversing context pointers.

Return <=400 words: concrete bypass/atomicity bugs with exact function/line and
a proposed falsifying test, or "no demonstrated bypass" with coverage limits.
Do not claim tests executed, historical behavior, legal certification or usage
savings. No code patch needed. Codex decides and verifies findings.
