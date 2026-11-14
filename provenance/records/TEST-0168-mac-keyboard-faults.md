# TEST-0168 - BootHush Mac keyboard fault injection

2026-11-14; SRC-0607 / SPEC-0100 / IMPL-0120. Eight keyboard scenarios, including
stall, absent reply, two delay boundaries, malformed byte followed by actual
press and release, and disconnect, each ran with and without a bounded witness.
The pinned original ROM, model and exact outcomes are in the
[receipt](../../docs/development/boothush-paired-20261114.md). The guest latches
timeouts; no physical-device or timeout-recovery claim is made.
