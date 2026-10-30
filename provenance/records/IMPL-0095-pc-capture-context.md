# IMPL-0095 - Read-only capture-context snapshot

2026-10-30; SRC-0565. Original Windows-only observation function takes an already selected owned window and PID; verifies its target owner before and after gathering; never selects or activates a window, sends input or controls a VM. A separately verified caller remains required and is not integrated. See the [receipt](../../docs/development/pc-capture-context-evidence.md). Not a GUI or release acceptance.
