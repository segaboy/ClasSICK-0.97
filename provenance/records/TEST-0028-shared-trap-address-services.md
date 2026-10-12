# TEST-0028 - Shared trap-address services verification

2026-10-12, SPEC-0020 / IMPL-0024 / ADR-0023. Initial status: verification
in progress, neither updated native image executed. Record later results in the
separate dated evidence; do not convert compilation into a boot observation.

Five hosted suites cover every 16-bit instruction word with an arithmetic
oracle, output guards, every directory entry and invalid 16-bit number, 24-bit
address/configuration bounds, independent tables, A0/SR/register preservation,
same-number formats, direct calls and restoring a patched service's own entry.
Two CTest object checks enforce no core imports/global state and only original
probe imports. Full existing matrix adds these seven checks in each configuration;
Clang/GCC debug twins, actual i686 and x64 sanitizers retained. Twenty additional
Clang/GCC O0/O2 width and ARM64 LE/BE compile/import audits distinguish builds
from execution. Both EFI and original 68000 images link the same core/probe.

Toolbox stack/auto-pop/callback ABI, arbitrary guest patch execution, low-memory
table exposure and complete SPEC-0017/history/physical/edition compatibility
are deliberately unimplemented and have explicit refusal results.

Corrected local verification passes 939 CTest checks in nine configurations:
117/117/117/90/117/102/102/102/75. Twenty extra object audits and Clang/GCC
service executable twins pass; Mac hosted verification retains 24 suites/four
zero-divisor controls. Normal/UD/GP EFI twins pass 26/27/27 corruption rejects
and twelve omitted-object rejects each, plus injection-profile refusals. Nine
media writers, thirty media corruptions and three writer refusals pass.
The final local chain log SHA-256 is
`475b5d9fc51bf6ca27461e7d89aedbf998721a47dc4530f501cf4796b21968df`.
The service observer separately passes five corrupted-result controls.

Four 68000 builds/twins reject seven corruptions/eight omissions per build.
Os uses 5,260 ROM bytes; O0 7,896. Both allocate 2,116 data/BSS bytes and keep
the 8-KiB stack. These remain unloaded build claims until separate observations.

Failure history: the additional Clang i686 O0 audit found compiler-generated
memcpy from aggregate copy/initialization. Replace those with explicit scalar
stores, confirm only original imports, then repeat the complete chain/native
builds in fresh directories. Provisional images are retained, never admitted.
One attempted object-audit retry refused an existing output directory as required;
the final fresh wrapper completed. No uncertain external implementation used.
