# IMPL-0058 / TEST-0066 - Original native window input owner

2026-10-21;SRC-0329..0331;SPEC-0055;ADR-0063.
Shared native C coordinates one pointer,counter,window and compositor. Completed
consumer stages are retained on fault;explicit repair finishes only pending
work,cancels and fully repaints. No replayed press,rollback,new heap allocation
or framebuffer. Owner224bytes ILP32/248 LP64,borrowed resources unchanged.

Copilot supplied345 proposed lines in148.19s,one frozen20,865-byte prompt read,
no edits/tests. Header and254-line core adopted after alias-order,art/pattern/
state validation and repair-token corrections. Its unfinished58-line test is
excluded. Lead's independent contract-first pixel/fault oracle supplies the
actual verification;no second provider or redundant implementation.

311 full Clang64 CTests plus2 sanitized/2 GCC32 focused tests pass. Each oracle
run checks63 complete images and565 assertions,including drag/clamp/final
release,held suppression,cancel,saturation,retained counter/window/render fault
prefixes,repeated cancellation failure,begin failure and explicit cursor rebind.
Four optimized Clang/GCC32/64 variants pass;two compiled defects are rejected.
12width/endian/EFI object audits pass. Native68000 CI pending at source commit.
[Exact evidence and source pins](../../docs/development/window-input-evidence.json).

No platform/boot/guest ABI binding,target run or release. Existing Mac M1-M7,
PC GUI/human,source/role/UART/historical/edition/physical/full128K/B2/native-app/
Linux/mini-vMac/both System1-ROM gates remain.24originals/held brief/old records,
released assets/tags and retired wiki preserved. Next:hosted adapter integration;
Mac source admission and fresh target-trial approval remain separate.
