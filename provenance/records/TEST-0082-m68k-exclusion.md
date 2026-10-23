# TEST-0082 - Isolated CPU exclusion verification

SPEC-0068 / IMPL-0069; SRC-0394..0398. Nine executed host audit-control methods
cover valid decoding, wrong mask, missing/reordered snapshot, constant SR
restore, extra SR write after a blank line, missing API, malformed syntax and
clobbers disguised in comments. Mathematical snapshot/LIFO model covers256
supervisor trace-off SR states. These do not execute the CPU callbacks.
An actual non-m68k compile is explicitly refused. Actual native six-object/twin/frame/
import/SR-operation checks pass in source-pinned Windows/68000 CI; largest
callback function frame 4 bytes. Own disassembly reviewed for token flow.
Lead controls follow primary read; no final author-blind claim. No actual
ordering/ISR/RTE/quiescence/latency, whole-stack/128K or target proof.
[Receipt](../../docs/development/cpu-c16-evidence.json).
