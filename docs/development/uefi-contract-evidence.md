# UEFI preboot contract verification

Date 2026-10-08. SPEC-0006 / ADR-0010 / IMPL-0010 / TEST-0012. Tested scope is
independent framebuffer/map/owned-span validation and an exit-outcome model.
It calls no firmware, dereferences no physical address and launches no VM.
[The native-PC profile](native-pc-profile.md) defines the proposed loader/device
ownership obligations and the unresolved exact firmware provenance/identity.

Repeat with prepared pinned tools and a fresh output directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Setup-Windows.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Setup-SecondCompiler.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-UEFIContracts.ps1 -BuildRoot C:\ClasSICK\uefi-contract-local -Sanitizers
```

Expected full protocol: 43 checks per five Clang configurations, 28 per four
GCC configurations; existing debugger/sanitizer/core/endian/link controls plus
ARM64 LE/BE preboot-object audits and two preboot-test executable twin hashes.
Current-core link fixture hashes remain separate from the new platform adapter.
Detailed fixture counts/limitations reside in TEST-0012. Final observed immutable
source, artifact fingerprints and CI results will be added after verification.
