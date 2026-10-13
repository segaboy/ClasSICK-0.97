# Typed register patch checkpoint - 2026-10-13

SPEC-0027 / ADR-0031 / IMPL-0030 / TEST-0035 / SRC-0121-0125.
Compiled source `5c40c6d7acb445d8f0ba8a27173c85af0d0b1b1f`. [CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38434766286) passes both jobs;
complete original68000 manifest matches local. Lead self-review, human provenance
review pending.

Installed guest patches now execute through an opt-in typed personality path.
All eleven selected memory routines can chain to their saved original identities.
Allocation flags follow the active signature. The original ten-byte patch calls
NewPtr, changes D1 and returns in four instructions; dispatcher saves restore
D1 while exposing D0/A0. Tail chaining takes two instructions. Get/Set still
observe and restore directory entries; independent runtimes remain independent.
Nested patches share ancestor budgets and reject a fifth service level.

The register wrapper restores A1/D1/D2 and bit8-controlled A0, caller PC/A5/A7
and the specified SR/low-word D0 conditions. Pre-entry refusal preserves state.
Late faults restore the complete caller frame while retaining already completed
guest data, allocation and directory changes; the test queries the surviving
allocation after the fault. A bad saved-original return refuses before allocation.
Legacy atomic dispatch still reports needs-execution for the same guest target.

The initial complete nine-profile matrix passes1416 CTests. After the debug
compiler padding-helper correction, all54 affected patch checks repeat on the
final source in those profiles; unaffected1362 results are retained. Clang/GCC,
32/64-bit, debug/release and validated sanitizers pass, with matching debug twins.
Ten extra patch objects pass helper/global import audits and both endian targets.
CI executes the entire final-source1416 plus340 bootstrap checks. GNU68000 passes18 module
audits, two complete relocatable links and an independent ten-byte assembler
comparison. Metadata costs100bytes; whole-closure frame maxima are608/604
bytes. These are partial costs, not measured cumulative stack or final128K proof.
Six prior EFI hashes and both own ROM hashes/input fingerprints remain unchanged.

Preparation failures are retained in the machine-readable record. The important
portability corrections remove compiler-generated memcpy and embed original
ordinary-user execution metadata after observed WinError740; renaming alone did
not fix that launch. No protection setting changed. Later matrices supersede
the incomplete early runs; no failed/not-run check is relabeled passed.

Only13 selected OS signatures and matching saved-original memory veneers are
admitted. Get/Set direct veneers, arbitrary direct guest calls, Pascal marshalling,
actual manager callbacks, CODE loading, QuickDraw and most CPU instructions remain
open. Six research originals are unchanged/untracked. No new boot binding,
VM/emulator/reference run, native application/SDK or UART body work. Historical,
physical, parity, three editions, full budgets and B2criterion6 remain open.
Git/HQ/wiki evidence only; no delegation, chats, automation or OneNote.

[Machine-readable evidence](guest-patches-evidence.json). Next: the finalized
bounded resource-fork reader contract, as a CODE-only loading prerequisite.
