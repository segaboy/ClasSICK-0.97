# IMPL-0117 - Original 286 capability boot diagnostic

2026-11-13; SRC-0574 / SPEC-0087 / IMPL-0098 / TEST-0164. Lead-authored
`probes/pc286/boot-span-cap.c` and `scripts/Test-BootHushPC286Capability.py`
run the existing original capability model in a bounded 16-bit BootHush
fixture. GPL-3.0-or-later. Fixed far-memory windows are not a production
provider. No external implementation, reference media or worker code was
used. [Inputs, outcomes and limits](../../docs/development/pc286-capability-boot-20261113.md).
