# TEST-0085 - Mac RAM bootstrap checks

172 real-graph checks, five fault-prefix cases and exact initial PC canvas FNV32=2123700856 pass. Clang/GCC and ASan+UBSan checks pass. Original author misaligned-hook access reproduced under UBSan and excluded. Native layout compile is pending until CI; no hardware/boot claim.

[Combined evidence](../../docs/development/release2-integration-evidence.md).

Follow-up: the clean Clang350-test regression passes; focused GCC Release and
Clang ASan+UBSan each pass10 tests. Three compiled overpublication/centering/
inverted-preflight defects are rejected. Native CI40020378727 stops in the
bootstrap compile. GCC -m32 independently reproduces a Wtype-limits error in
the widened capacity comparison. Lead gates that check with SIZE_MAX greater
than UINTPTR_MAX, keeping the check only when meaningful; the32-bit compile
then passes. The verifier now prints captured compiler diagnostics on failure.
This correction changes no PC image bytes; native CI remains pending.
