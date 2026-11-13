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

The `--all-directions` extension to the original Mac timing probe was run:
four 4,096-cycle cases passed and four 2,048-cycle cases each moved one pixel
opposite the requested direction with 29 omissions. These are BootHush model
observations; no physical mouse timing or root-cause conclusion follows.

2026-11-13: Both Mac scheduling modes passed four checkpoints and replay;
the readiness-relative mode compared two simultaneous memory spans. The fast
Mac timing failure persists. The 286 span diagnostic and three negative
controls passed with the full payload and both banks inspected at one stop;
the prior single-range mode also passed. The x64 media verifier accepted the
matching payload and rejected a mismatch but did not execute it. [Identities,
results and limitations](../../docs/development/boothush-paired-20261113.md).
