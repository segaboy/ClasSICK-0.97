# ADR-0045 - Shared native button rule before controller routing

Accepted2026-10-16; SRC-0227..0232; SPEC-0039 / IMPL-0043 / TEST-0047.
Use one original allocation-free C button transition rule on native68000 and
native Windows/x86. Keep the pure three-byte report decoder independent of the
existing keyboard controller driver. Do not add a competing8042 poller: later
startup/AUX routing requires a reviewed single-owner controller contract.
Strict tagged reports, header-controlled signed deltas, whole-overflow drops
and no-output refusals are original engineering choices. Keyboard FIFO remains
keyboard-only. Older Mac ROM entries/loop/ABI and released assets stay intact.
Windows capture cancellation clears held state without inventing a press;
capture notifications never recapture. Counter is a saturating keyboard/mouse sum.
OneNote and its Git wiki queue are permanently retired by the owner; ordinary
development/provenance/HQ records remain. No new emulator/VM/hardware/reference
trial or release. [Evidence](../development/mouse-input-evidence.md).
