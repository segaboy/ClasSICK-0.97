# TEST-0162 - BootHush Mac mouse observations

2026-11-12; SRC-0601 / SPEC-0096 / IMPL-0115. BootHush's own `mac_live`
fixture reported a pass for message-injected corners, drags and click. Live
automated mouse movement/clicks were captured and deterministically replayed;
the attempted host drag lacked a recorded primary-button event. The original
lead probe reproduced timing-dependent wrong or short movement; its nonzero
result is an actual failure, not a passing release check. Hash refusal and
no-overwrite controls passed. [Exact evidence, diagnostics and limitations](../../docs/development/boothush-mac-mouse-20261112.md).
No physical or human validation, historical claim, or Mac release qualification.
