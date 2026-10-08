# UEFI loader verification

Date 2026-10-08. SPEC-0007 / ADR-0011 / IMPL-0011 / TEST-0013.
Actual original x64 firmware-call orchestration is tested with original hosted
callbacks. Original EFI entry/stack/stop code is linked and inspected, never loaded.
No external firmware has been adopted or run. B1 remains separately passed.

Repeat with the pinned prepared tools and a fresh output directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-UEFILoader.ps1 -BuildRoot C:\ClasSICK\uefi-loader-local -Sanitizers
```

The wrapper retains TEST-0012's complete core/hosted/debugger/sanitizer/link/endian
protocol, adds x64 loader behavior/object checks and hosted Debug twin hashes,
then builds/audits four native EFI images and packages the original O2 payload.
An inspection-only image replay is available through Verify-UEFIImage.ps1 with a
separate fresh output path. It executes no generated EFI file or privileged code.

Final immutable source/result fingerprints and observed CI will be recorded here
after the final matrix. Development passes are not that final result. Existing
twenty-three named fingerprints plus two loader executables/two EFI profiles give
twenty-seven comparisons when all are emitted and verified. Equality is scoped
reproducibility, not provenance, reference compatibility or native boot proof.

[Firmware review](firmware-eligibility.md) now identifies the actual x64 producer
and selected module declarations; installed correspondence/transitive derivation
closure remain needs-review. No source bodies/firmware were adopted or inspected.
The payload is a directory tree, not a formatted boot disk. Own exception state,
native drivers/event loop and observed B2 are still required. Macintosh/mini vMac
replacement startup and physical three-edition gates remain separate.
