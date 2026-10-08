# TEST-0013 — Hosted UEFI loader callbacks and unloaded EFI inspection

Date 2026-10-08. SPEC-0007 / ADR-0011 / IMPL-0011, original fixtures only.
No firmware execution, physical address access, VM launch or OS boot.

Protocol: Verify-UEFILoader.ps1 extends the complete TEST-0012 protocol. x64
adds four behavior suites and one optimized loader-object audit: Clang x64 Debug
twins/Release/ASan+UBSan, GCC x64 Debug twins/Release. The actual i686 profiles
retain portable preboot checks and exclude the x64 ABI. Prior debugger, sanitizer
detection, endian compile-only and core-link controls continue independently.

Loader checks: 360 independently evaluated CRC tables plus literal 123456789
vector; 27 finite success/stale/failure traces; protocol GUIDs and all ABI arguments,
including six-argument OpenProtocol; map key zero/full-width keys; cached callbacks
after service/protocol fields are destroyed; exact service order and cleanup;
malformed headers/image/graphics/maps; too-small/oversized capture and failed
cleanup; retry capture failure with no return/free; seven/eight image-span bounds,
coverage and rejection stability; independent executions and allocation guards.
Assertions remain active in Release. Mock physical image/framebuffer are not read.

Four own EFI images: Clang/LLD x64 O0/O2 fresh twins, closed original C/assembly,
no default libraries. Bounded original PE/map audit checks all 15 required symbols,
actual entry, section/raw extents/permissions, zero other directories/imports,
DIR64 records/anchor and exact stack/call/halt bytes. Independent LLVM header,
import/relocation inspection and disassembly are retained outside Git. Twelve
deliberate image corruptions and a real omitted-transition link must reject.
The audited O2 copy becomes EFI/BOOT/BOOTX64.EFI in a payload tree, not a FAT disk.

Initial hosted five-check development pass and initial four-image development
pass precede the final full protocol. First native link used unsupported numeric
subsystem spelling; the named EFI application spelling succeeds. First PowerShell
auditor parse failed multiline operator placement, corrected before acceptance.
These were build-tool/script mistakes, no native execution or altered firmware.

Final source `c912f2f4db67241c8114cc5415bb16ab873e1fc2`; fresh complete local
wrapper exits zero. Clang x64 48 checks in four configurations/i686 43, scoped
GCC x64 33 in three/i686 28; all prior controls and four EFI images pass.
Twenty-seven named pairs match; all earlier twenty-three fingerprints are preserved.
The payload copy matches audited O2. Artifact fingerprints/local results are pinned
in [the snapshot](../../docs/development/uefi-loader-evidence.md); remote pending.
Human provenance review, real firmware ABI/relocation/exit/owned-stack execution,
exceptions/NMI, paging/devices/B2 and all historical/physical gates remain open.
