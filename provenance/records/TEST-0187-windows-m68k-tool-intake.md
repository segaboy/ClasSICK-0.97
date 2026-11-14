# TEST-0187 - Windows 68000 toolchain intake controls

2026-11-14; SPEC-0106 / IMPL-0133. A harmless Windows PE fixture exercises
intake and verifies refusal of a wrong target, malformed executable and
missing tool. The window builder refuses a wrong target before creating an
output directory, and a fixture that cannot compile does not produce a ROM
manifest. This is not a successful 68000 build or release qualification.
[Evidence](../../docs/development/windows-m68k-toolchain-20261114.md).
