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

Native follow-up: three isolated PC starts and two fresh Snow 128K starts pass
the same ten-check C probe. [Evidence](../../docs/development/shared-services-evidence.md)
distinguishes native C service execution from still-unimplemented A-line/legacy
application execution. PC post-second-60 registers and forty own Mac diagnostic
bytes agree on zero service error/signature; both Mac runs reach stage4/ticks3610,
one TRAP return/expected seal/zero fault. PC VM off/unchanged VDI, both Snow
windows gone/normal loop-exit logs; no process-exit-code claim for Snow.
CI 38257185223 passes both jobs at `02e2f5a6daf29a1fd4dc7a61ea460d371fa493ee`;
six EFI/two ROM profiles and ROM source fingerprints match local admission.
