# TEST-0042 - Guarded single-instruction conformance

SPEC-0034 X-1 / IMPL-0038 / SRC-0184..0186; 2026-10-15.
Original tests/m68k-guard.c, pure immutable native deny/allow rules and independent
CPU/RAM/gateway snapshots. Five suites: events, atomicity, control, services,
aliases. Successful twins prevent an always-deny implementation passing.

Actual opcode/whole four-byte extensions/data read/write/target/class/commit,
late source increments, withheld stores/flags/A7/PC, register-only MOVEA A7,
S/T/I versus CCR, strict24 and map permissions, BSR/JSR/RTS stack/target denial,
all4096 A-lines before services, retained prior NOP, envelope/output/context/
backing aliases and no-report-write native refusal. No mutable policy logger.
Existing unguarded interpreter suites remain required regression coverage.
Tests run as hosted original instruction fixtures, not an application image,
session bridge, emulator or historical reference. Complete compiler/regression,
object/link/CI results and pre-final failed attempt are recorded in evidence.
No historical, boot/physical/full128K/parity/edition/native-app/B2 promotion.
