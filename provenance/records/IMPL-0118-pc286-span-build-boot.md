# IMPL-0118 - Original 286 span-construction boot diagnostic

2026-11-13; SRC-0536..0539 / SPEC-0082 / TEST-0165. Lead-authored
`probes/pc286/boot-span-build.c` and `scripts/Test-BootHushPC286SpanBuild.py`
execute the existing original span planner/binder with the pinned 16-bit
compiler and fixed original BootHush windows. GPL-3.0-or-later. No production
memory ownership, full OS or hardware claim follows; no external implementation
or reference media was used. [Evidence](../../docs/development/boothush-developer-report-20261113.md).

Later 2026-11-13: The diagnostic additionally binds planned spans into the
existing checked access model and exercises cross-span/bank transfers and
refusals. [New evidence](../../docs/development/boothush-developer-report-20261113-b.md).

Later 2026-11-13: Added an original planned-span-to-capability guest diagnostic
and expanded the original diagnostic floppy loader to two tracks.
[Evidence and limits](../../docs/development/boothush-developer-report-20261113-c.md).
