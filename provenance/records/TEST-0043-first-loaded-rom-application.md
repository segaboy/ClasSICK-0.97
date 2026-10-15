# TEST-0043 - Original loaded application integration

SPEC-0035 / IMPL-0039 / SRC-0188..0190; 2026-10-15.

Tests use the actual original resource parser, loader, owner, memory manager,
guarded interpreter and retirement path. Successful result97/pattern5A5A5A5A,
five steps, observed disposal/scrub/world state, original caller frame and separate
stop snapshot, retained stack, repeated fresh generation and unchanged base flags.
Original malicious instruction bodies go through PREPARE/LOAD too: traps/F-line,
above-A5/low-memory/token accesses, forbidden row targets, invalid A7, wrong-stack
RTS token. No post-LOAD native patch substitutes for the loaded input program.
Budget exhaustion retains earlier writes and prohibits resume/second run.
Token/context/generation/map mismatches refuse before a guest effect. Native output
alias refuses with report bytes preserved. Corrupt protected world quarantines
retirement without partial scrub. Pixel oracles check actual result/phase markers,
error presentation and framebuffer tail canaries; preview is hosted, not a boot.

Six behavioral suites plus new run-object audit are included in complete compiler/
regression/CI checks. Underlying original image post-unit-fault tests remain
required regression; new wrapper RELEASE injection is not independently claimed.
ROM twins and independent assembly encoding/static layout/stack/seal/link audits
are compiler evidence. Actual emulator/physical boot is a separately authorized
observation, not implied by source, host preview or test counts.
