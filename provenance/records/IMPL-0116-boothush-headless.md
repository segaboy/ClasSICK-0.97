# IMPL-0116 - Original headless BootHush diagnostics

2026-11-12; SRC-0602 / SPEC-0096 / SPEC-0069 / TEST-0163. Lead-authored
`scripts/Test-BootHushMacWindow.py`, `scripts/Test-BootHushPC286BankPlan.py`,
`scripts/Test-BootHushPC286SpanAccess.py`, and original boot diagnostics
`probes/pc286/boot-bank-plan.c` and `boot-span-access.c`. The 286 scripts use
an original generated diagnostic loader and existing separately linked project
models; generated images remain outside Git. GPL-3.0-or-later. BootHush is
used through its documented black-box interface. No Apple, Microsoft or
emulator implementation code or reference media was inspected or reused.
[Evidence and scope](../../docs/development/boothush-headless-20261112.md).
