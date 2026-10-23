# ADR-0076 - Require measured native performance on each target

2026-10-23; SRC-0383..0385. Accept the owner's native performance priority for
Macintosh 128K/68000, proposed PC/AT 286, and native x86-64 UEFI. Preserve common
behavior and portable C semantics while allowing separately reviewed original
architecture/platform fast paths, including assembly when measured benefit
justifies it. No particular implementation, CPU feature or ABI is authorized here.

Portability is not an exemption from fixing demonstrated avoidable overhead;
global optimality and zero overhead are not established promises. Require
same-workload baseline/candidate evidence, semantic/fault conformance and honest
speed/space/latency tradeoffs for the relevant target. Do not infer physical
performance from hosted tests or emulation alone. Existing paired release order,
clean-room/native-app/source/target-run and publication gates remain unchanged.
[Plan](../development/native-performance-plan.md).
