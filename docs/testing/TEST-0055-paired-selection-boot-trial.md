# TEST-0055 - Paired selected-port boot observations

Executed2026-10-18 local /2026-10-19 UTC; SPEC-0047 / IMPL-0049; SRC-0278/0279.
One Snow Early128K boot and one isolated native x86-64 EFI64 boot, exact previously
verified original ROM/EFI/disk; no rebuild/release. Identity/configuration checks,
independent media reader, VDI/raw roundtrip, Mac two-click and PC press/held/release/
Space checks, six static framebuffer comparisons, one normalized same-counter
cross-target comparison, PC guest-second60 normal result, closure and VM inventory
restoration passed. Persistent Snow preference isolation is unqualified; only the
explicit fresh workspace was verified. Inputs were synthetic, not human/physical.
Mac empty versus PC timed progress strip remains an explicit carrier difference.
[Actual evidence and limits](../development/paired-selection-boot-trial.md).
