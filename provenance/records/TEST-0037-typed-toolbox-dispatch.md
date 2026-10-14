# TEST-0037 - Typed Toolbox dispatch conformance

SPEC-0029 / ADR-0033 / SRC-0139-0141. Implemented original hosted tests,
2026-10-14. Seven suites plus a freestanding check; [executed evidence](../../docs/development/toolbox-evidence.md)
records exact revisions, matrix counts, final correction and compiler/CI results.
Claude research plans remain unexecuted and are not these original tests.

Original suites cover canonical table admission and immutable identity/config checks; all scalar
widths, declaration order, bool/char encodings and empty/result frames; pointer
alignment/permissions/end bounds/string length/full-span and pairwise/stack aliases;
exact staged output counts, result/output/scratch commit and atomic refusals;
normal and checked procedure auto-pop, forbidden function/token auto-pop;
per-routine register and continuation refusal independent of bit11; patched,
mismatched and direct identity refusals; root ExitToShell/last-budget attempt,
inactive/interrupt/returning-callback exit refusal; retained prior guest effects,
fault precedence, caller restoration, runtime reuse and two independent instances.

Independently authored interpreted push/trap/pop/RTS and explicit-exit bytes have
assembly counterparts and checked encoding/stack/step observables. They are own
core fixtures, not native applications, CODE files, manual samples or Claude's
unexecuted research cases. Strict hosted compiler/width/optimization/sanitizer,
freestanding/import/endian and complete GNU68000 closure/storage/frame audits.
No boot integration, VM/emulator/reference run or historical/edition/budget claim.
