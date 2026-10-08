# ADR 0012: Owned x64 terminal exception state

- Accepted scoped original design, 2026-10-08; owner SRC-0042 continuation.
- SPEC-0008 / IMPL-0012 / TEST-0014; human provenance review pending.

The next native-PC startup dependency is owned CPU fault state. Put descriptor
serialization and privileged assembly in platform/pc/x64, preserving the portable
core. Use the reserved bundle tail and four bounded IST stacks, with a single
first-fault record and terminal halt. No new firmware/runtime dependency.

Independent hosted byte tests and unloaded image audits establish only layout and
linkage. Keep the installation window/nesting/mapping assumptions explicit and
require real fault injection before hardware acceptance. Firmware provenance and
all B2 devices/boot gates remain open. This advances startup without changing the
three-edition contract or the required real Macintosh/mini vMac goal.
