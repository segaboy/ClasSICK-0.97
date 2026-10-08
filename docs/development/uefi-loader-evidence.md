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

Compiled/test inputs: `c912f2f4db67241c8114cc5415bb16ab873e1fc2`.
Final verification-script source: `97b10493955f0e5b5331f61dd703c939b0566524`, with
those C/assembly/fixture bytes unchanged. Fresh final local
`C:\ClasSICK\uefi-loader-verification-20261008-b` and adjacent `.log` pass both
script success and final native exit zero under Windows PowerShell 5.1. Earlier
full `a` and development passes precede the expected-control status correction.
All 27 fingerprints are identical between `a` and final `b`.

Observed local matrix: Clang x64 48 checks per Debug twin/Release/ASan+UBSan,
actual i686 43; scoped GCC x64 33 per Debug twin/Release, actual i686 28, Windows
adapters disabled. All prior debugger/sanitizer/core/link/endian controls pass.
Four EFI images pass: O0/O2 twins, 15 original symbols, two DIR64 relocations each,
zero imports/libraries, exact stack/halt bytes. Twelve image controls and a real
omitted-transition link reject. The original O2 payload copy matches its audited file.

| New artifact/profile | Matching SHA-256 |
| --- | --- |
| Clang hosted loader Debug | `e824ca9e6e5a66487d2afeffdea799dd084cd12bd41a00c2c7581dd88306add1` |
| GCC hosted loader Debug | `061008cff08952cc976081b1ea1214cacc7f74878ac47e404b2410d9afed3113` |
| Clang x64 EFI O0, 11,264-byte file | `b99e12c7ea9de61207374d160aab6b53a813c751b601dafcde46cde9132db325` |
| Clang x64 EFI O2, 7,168-byte file | `b2702e772a2711064009cc37e01f0d896a9df033e118508b4fbbec60fbbd0299` |

All earlier twenty-three fingerprints are unchanged against the prior final local
log, not inferred from partial documentation tables. With the four above the wrapper
emits twenty-seven named comparisons. Equality is scoped reproducibility, not
provenance, reference compatibility or native boot proof. First CI 37788994328 at
c8a2342 passes all checks/27 fingerprints but fails final status propagation from
the deliberately rejected transition link. TEST-0013 records the correction.
Corrected [Windows CI 37790529852 passes](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37790529852)
at evidence source `1acd2850fd1e6c6d408ac2205a002dc8048d41b2`: 182-file guard,
pinned tool preparation, bootstrap twins and full wrapper. Six 48-check Clang
result sets (including bootstrap twins), one 43-check i686, three 33-check GCC
sets and one 28-check i686 pass. All 27 named fingerprints equal final local `b`.
All debugger/sanitizer/link/endian/image/control checks pass; no generated EFI
code executed. This result update changes documentation only, tested inputs unchanged.

[Firmware review](firmware-eligibility.md) now identifies the actual x64 producer
and selected module declarations; installed correspondence/transitive derivation
closure remain needs-review. No source bodies/firmware were adopted or inspected.
The payload is a directory tree, not a formatted boot disk. Own exception state,
native drivers/event loop and observed B2 are still required. Macintosh/mini vMac
replacement startup and physical three-edition gates remain separate.
