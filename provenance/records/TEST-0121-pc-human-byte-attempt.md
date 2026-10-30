# TEST-0121 - User-directed PC byte diagnostic attempt

2026-10-30; SRC-0573. One isolated original PC sequence-recorder candidate was resumed after user readiness. The user reported capturing the mouse and moving it; guest frames showed the pointer at the lower-right edge. The paused RIP did not match the recorder stop, so its registers were not interpreted as a sequence; the number of bytes delivered is unknown. The user did not perform a drag. The original VM, existing inventory and media were preserved. [Canonical observation](../../docs/development/pc-sequence-evidence.json).
