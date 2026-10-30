# IMPL-0097 - Read-only owned-window context caller

2026-10-30; SRC-0568. Fresh worker's original caller binds to a generated owned-target identity and performs process, path, executable hash, desktop and window checks around the read-only context snapshot. It does not call the input or focus-changing paths. Output is invalid if its exit status is nonzero. [Evidence and limits](../../docs/development/pc-capture-context-evidence.md).
