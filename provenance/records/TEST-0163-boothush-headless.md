# TEST-0163 - BootHush headless Mac and 286 diagnostics

2026-11-12; SRC-0602 / IMPL-0116. Four injected-window Mac checkpoints and
the final record/replay comparison passed; fast or unguarded timing cases still
fail. Separately compiled pinned 16-bit 286 bank-plan and far-span diagnostics
booted under BootHush's built-in firmware. The bank-plan baseline and two
negative controls passed; the far-span baseline and three negative controls
passed, including separate code/bank observations with matching stop cycles and
unchanged floppy input. Wrong-ROM and existing-output refusals passed. These
tests do not qualify human input, a production memory provider, hardware, an OS
boot or a release. [Methods, identities and limitations](../../docs/development/boothush-headless-20261112.md).
