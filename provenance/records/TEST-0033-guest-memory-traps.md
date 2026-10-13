# TEST-0033 - Built-in guest memory trap conformance

2026-10-13; SPEC-0025 / SRC-0110/0111 / IMPL-0028.
Original own-core tests and compiler audits, not historical/native execution.

Five hosted suites cover bindings, routing, failures, data addressing and the
original guest program. Freestanding import/global checks add a sixth CTest.
Thirty-four accepted word/flag/A0 combinations compare register marshalling and
heap state with the independently tested SPEC-0023 adapter while checking CPU
continuation and six-byte exception publication separately. Initialization
failures preserve the old bindings/table/manager/RAM. Get/Set patches and direct
saved-original calls preserve logical identity; incompatible signatures refuse.

Failure snapshots compare frame, directory, manager, all guest RAM/stack and
code. Include malformed fetch/mode/stack, wrong signatures, unavailable code,
unsupported flags, explicit interrupt, invalid manager/globals, scratch inside
either heap or low-memory prefix, and overlapping BlockMove destinations.
API errors still execute and advance PC. Address tests sweep all256 tag bytes,
three widths and read/write directions, preserving full registers. Odd, end-of-
space and strict PC/RTS/BSR-stack paths fail without mutation; tagged trap
arguments retain the strict Memory Manager contract.

The 82-byte independently authored program performs35 steps and15 real memory
traps, uses a flag-bearing locked master pointer through the data bus, resizes
while unlocked, preserves payload across movement, copies, shrinks a system
pointer, frees both objects and returns to an owned sentinel. Expected signature
47545231, all eleven operations covered. It is a hosted core fixture, not an
x86 application or a guest callback invoked by the system.

Run scripts/Verify-MemoryTraps.ps1 with pinned Clang/GCC/validated sanitizers,
then scripts/Verify-MemoryTrapsM68k.py for original standalone GNU68000 linkage,
frame reports, binding storage and independent assembly encoding. Source/input
hashes and quantitative outcomes are retained in the development evidence;
compiler frame maxima are not measured stack high-water or final ROM/RAM proof.
No VM/emulator launch or historical/native application acceptance.
