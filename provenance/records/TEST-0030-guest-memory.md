# TEST-0030 - Checked guest memory execution verification

2026-10-12; SPEC-0022 / IMPL-0026. Local verification passed.

Independent branch truth masks exhaust sixteen instruction conditions across all
sixteen N/Z/V/C combinations. Width/register/operand tests, source-before-dest
auto-address effects, A7 byte increments, MOVEA word sign extension, access and
permission edges, stack/target and extension failures, and stable frame/gateway/
buffer checks use original fixtures. The separate common program checks twelve
steps, BSR/RTS stack behavior, data changes, flags and sentinel preservation.
Standalone objects must remain free of runtime imports under both compiler
widths/optimizations and both ARM64 byte orders. No hosted result is a new native
boot or legacy application observation.

Full Windows chain passes 1,110 CTest checks in nine configurations:
136/136/136/109/136/121/121/121/94, including the twelve added suites/audits
per configuration. Thirty extra new standalone object audits pass for Clang/GCC
32/64-bit O0/O2 and ARM64 little/big-endian. Both new test executables have
matching Clang/GCC debug twins. Existing service/CPU/media/EFI/Mac controls pass.
Local log SHA-256 c74c0a6a4d237d0e5da8141abc6300b3fab12ff0dcbc1757f52fbef9ab2e07ff.

Pinned GNU 68000 GCC13.3 at O0/Os passes ten source-object import/global audits
and two complete relocatable core links with no unresolved runtime symbols.
The independent assembler emits exactly the 36-byte C fixture, SHA-256
0d8dcf6fb7e0a4f4edf9185836927be74ac833e7156c2a86a457234a178abd01.
Function frame reports are bounded; they are not a native stack high-water
measurement. Four existing ROM twins and all existing negative controls pass,
with the same Os/O0 image hashes as the previous admitted A-line checkpoint.
All six existing EFI profiles and normal raw media hashes are also unchanged.
The new module is not yet linked into either boot entry and no emulator was
launched for this checkpoint. CI comparison remains a separate follow-up.

The first focused command built only the two executable targets: nine behavior
suites passed, while three object audits reported their unbuilt object files.
Building those explicit object targets made all twelve pass, without a source
fix. The subsequent full fresh chain built and checked every target. An initial
register-prose locator was corrected to published 1.7.1/2 before publication.
The earlier standalone manifest is retained; final manifest adds all dependency
headers/tool lock and the corrected specification fingerprint.

Publication follow-up: CI38292360762 passes both jobs at compiled source
61c6c8de0963ee90748d284dab26b19727563e26. Combined 68000 objects, assembler
fixture, source fingerprints and every frame value match local. The frame-list
order differed by filesystem iteration; normalized comparison passes, without a
raw manifest byte-identity claim. Six EFI/two ROM profiles and ROM source inputs
match local/prior admitted images. No new native launch or application build.
[Detailed report](../../docs/development/guest-memory-evidence.md).
The owner confirmed the separate native-application gate in SRC-0099; records
and the 68k runtime plan add no native application/loader/SDK implementation.
