# IMPL-0013 — Native framebuffer presentation and original boot scene

Date 2026-10-08. SRC-0044 / SRC-0034; SPEC-0009 / ADR-0013 / TEST-0015.
Claude (Anthropic; session configured as `claude-opus-5-5`) assisted with the
original C, CMake, PowerShell, tests and documentation. Implementation lead
self-review; human provenance review pending. GPL-3.0-or-later.

Paths: platform/pc/framebuffer.h/.c; apps/boot-scene/scene.h/.c; platform/uefi/
native.h/.c and entry.c; tests/native-framebuffer.c, boot-scene.c, native-present.c,
scene-oracle.h; scripts/Check-ObjectImports.cmake, Verify-NativeFramebuffer.ps1,
Verify-UEFIImage.ps1 and Test-UEFIImage.ps1; CMake, workflow and records. Existing
core surface/arena, preboot, loader and descriptor algorithms are unchanged.

Original design: the presenter revalidates both descriptors, clips with 64-bit
intermediates and writes four volatile bytes per pixel, never reading the
framebuffer. The scene paints 32 half-open rectangles through `cs_surface_fill`
after translating them by the band top. Staging is one arena span of at most
64 KiB. The native gate checks the exit phase/status and the ready marker before
validating the framebuffer, then writes a fixed little-endian trace record. Native
stop now calls this gate after descriptor installation and still halts afterwards.
No switch tables, pointer tables or mutable globals are used, so the EFI image keeps
three DIR64 relocations and one writable 8-byte slot.

Exposure: the internal specifications above and SRC-0034's previously reviewed
UEFI GOP declarations; see SRC-0044 for the failed re-retrieval and unopened
search results. No other OS, firmware, sample loader, external header or graphics
library was consulted. No Apple source, ROM, executable, disassembly, decompiled
output, copied asset or reference image was used. The scene is an original
geometric test pattern.

Development iteration used unpinned Ubuntu GCC 13.3 / Clang 18.1 / LLD 18 in the
lead's Linux workspace: hosted CTest, GCC ASan/UBSan and a dev-only cross EFI link.
Those dev builds are not evidence. Pinned Windows results are in TEST-0015.

Unverified: real framebuffer mapping, caching and visible output, write speed,
the timed loop, keyboard, diagnostics, firmware eligibility, media and B2. The
presentation path has never executed natively.
