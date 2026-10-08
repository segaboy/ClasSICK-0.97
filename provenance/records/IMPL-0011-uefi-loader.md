# IMPL-0011 — Original x64 UEFI loader and owned transition scaffold

Date 2026-10-08. SRC-0037–SRC-0041; SPEC-0007 / ADR-0011 / TEST-0013.
Codex GPT-6 AI-assisted original C/assembly/CMake/PowerShell/documentation;
lead self-review, human provenance review pending. Project GPL-3.0-or-later.

Paths: platform/uefi/abi.h, loader.h, loader.c, entry.c, transition.S;
tests/uefi-loader.c; scripts/Check-LoaderObject.cmake, Test-UEFIImage.ps1,
Verify-UEFIImage.ps1, Verify-UEFILoader.ps1; CMake/workflow and linked records.
SPEC-0006 validators/model and portable core algorithms remain unchanged.

Original minimal ABI declarations carry static offsets/sizes. Original bitwise
CRC zeros the checksum field; bounded descriptor intersections derive owned image
spans. One fixed bundle reserves map/header/stack/fault/arena/trace storage. Cached
callbacks and owned metadata sustain retries without consulting firmware tables.
Hosted orchestration reports disposition; the native wrapper switches stack after
any attempted exit and never returns to firmware. Assembly/entry are inspected
only. No third-party header, firmware/loader library, startup/runtime or asset.

Mock firmware is independently authored, with argument/order tracing, known CRC
vector and a different-direction polynomial oracle, metadata corruption, truthful
allocated storage/guards, finite outcomes, poisoned callback/protocol tables after
an exit attempt and independent bundles. Hosted malloc/stdio are test-only.
Original PE auditor reads bounded bytes and maps; own image disassembly is allowed.
External uncertain firmware code/binaries are not inspection inputs.

Public UEFI/PE prose/declarations and adjacent examples were visible; no example
implementation adopted. Intel instruction interface/pseudocode is ISA knowledge.
Oracle build/license declarations/path names inform eligibility only; selected
module implementation bodies were not fetched. No Apple implementation/executable,
ROM, disassembly, decompilation, copied pixels or protected reference input.

This links an actual EFI entry, but has not run on firmware or native hardware.
It proves neither firmware honesty/page permissions nor owned exceptions/devices,
B2, historical Macintosh behavior or any edition boot. Firmware eligibility stays
needs-review. Compiler scope and immutable results are recorded in TEST-0013.
