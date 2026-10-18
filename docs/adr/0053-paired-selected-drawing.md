# ADR-0053 - Native selected-port visual parity

Accepted2026-10-18; SPEC-0047 / IMPL-0049 / TEST-0054; SRC-0274..0277.
Two real prepared native ports,one shared workload and one presentation union.
Use the original pure selection and view cores directly on68000/x86. This keeps
the PC OS feature independent of interpreting68k application instructions. The
separate SPEC-0046 legacy owner binding remains tested. Explicit build modes
preserve existing demos; product renderer and native input/controller unchanged.
Caller-owned workspace avoids large Mac stack frames; no pointers persist.
Compiler candidates and hosted previews do not imply boot observation or release.
[Evidence](../development/paired-selection-evidence.md).
