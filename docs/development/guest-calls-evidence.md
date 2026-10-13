# Bounded returning guest-call checkpoint - 2026-10-13

SPEC-0026 / ADR-0030 / IMPL-0029 / TEST-0034 / SRC-0116-0120.
Compiled source `cd71fc8350958f1e53795e22b062c9a8dbf468dd`; lead self-review, human provenance review pending.
[CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38415231217) passes both jobs; the complete native compiler manifest matches local.

The checked bus interpreter now executes canonical JSR/JMP control modes. An
original20-byte routine returns in six instructions through JSR/RTS and a
PC-relative JMP, with D0=17 and D1=42. The ordinary proc0 call service restores
caller PC/A5/A7, bounds its stack and active depth, and charges all ancestor
budgets. RTS/stack checks reject forged or unbalanced completion. Traps and
nested calls execute; interrupt/nonreturning/unknown signatures refuse.
Late failure restores caller registers while retaining completed guest memory
and trap-directory effects. A nested request with an already exhausted ancestor
now refuses before its token write, a separately checked pre-entry guarantee.

The initial full nine-profile matrix passes1362 CTests. After review found the
ancestor-entry edge case, all63 affected call checks were repeated on the final
source in those profiles; unaffected1299 results were retained. CI executes the
entire final-source1362 matrix plus328 bootstrap checks. Both compiler families,
32/64-bit profiles, validated sanitizers and matching debug executable twins
pass. Ten extra final call-object audits cover Clang/GCC32/64/O0/O2 and both
endian targets, with no unapproved helper import or mutable global data.

GNU68000 O0/Os passes ten module audits and two complete relocatable links.
Independent assembler bytes match all20 fixture bytes. Runtime metadata costs92
bytes; whole-closure compiler frame maxima are228/252 bytes. These are not
measured cumulative stack high-water or final linked-ROM/128K budgets.
Opcode tests cover128 EA fields,56 legal cases and72 refusals, plus256 independent
indexed cases, signed/odd/tagged/permission/stack boundaries and retained late
effects. Four service levels refuse a fifth; nested attempts cannot reset their
ancestor's budget. Existing six EFI hashes and both own ROM hashes/input
fingerprints remain unchanged. Older native observations retain their images.

Only ordinary no-parameter/no-result returning execution is implemented. No
manager callback installation, parameter/result marshalling, guest patch routing,
application loading, QuickDraw implementation, new boot entry or VM/emulator
trial. The six research drafts remain unchanged/untracked. Native application
gate, UART separation and all source/physical/historical/parity/edition gates
persist. B2criterion6 and final128K budgets remain open; no OneNote operation.

Retained preparation results: the first fixture used an invalid gateway RAM
span and was corrected; an initial compiler-root path was wrong and corrected.
The first native call manifest predates the ancestor fix and is superseded.
Final affected checks and final native artifacts above are the accepted results.
No prohibited material was used or source rule waived.

[Machine-readable evidence](guest-calls-evidence.json). Next: separately reviewed
typed guest patch dispatch, signatures/marshalling and CODE-only launch prerequisites.
