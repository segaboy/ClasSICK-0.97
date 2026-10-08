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

Final compiled/test/script source: `02160bc451d69cf78dd6149567ae54969bcee09f`.
The local final fresh output `C:\ClasSICK\uefi-contract-verification-20261008-c`
and adjacent `.log` exited zero under Windows PowerShell 5.1. Earlier development
and full a/b runs pass but precede the final fixture-only truthful-array correction.
No validator/core/interactive behavior changed in that correction.

Observed full protocol: 43 checks in each of five Clang configurations (Debug
twins, Release, actual i686 and x64 ASan+UBSan), 28 in each of four scoped GCC
configurations (Debug twins, Release, actual i686; Windows adapters disabled).
Existing sanitizer detection controls, freestanding core/import/data/endian
audits, six x64 LLDB sessions and three rejected launches pass. New UEFI-object
audits pass in each configuration and ARM64 LE/BE optimized compile-only audits.
No ARM64 execution claim; failed WOW64 debugging and broader GCC viewer limitation
remain unchanged. Fixture counts and failed exploration are recorded in TEST-0012.

Debug twin `classick_uefi_contract_tests.exe` SHA-256:

| Compiler | Matching SHA-256 |
| --- | --- |
| Clang 23.1.1 | `1ad1f49e96fb1681afc2847ae828c780b65e5fd4177bb36b280bddbf2ad12388` |
| GCC 16.2.0 | `1af0a4d6f6e72fcc2a04a66e5d43fb3e3da66fb86209c4b9e961e4a30bca636a` |

The prior twenty-one named Clang/GCC hosted/current-core link hashes still match
[the immutable link snapshot](https://github.com/segaboy/ClasSICK-0.97/blob/bbdeebb0a1e8eedabd2994b5162774427f65d0a8/docs/development/core-link-evidence.md).
Sixteen standalone images/eight profile twins, four missing-helper links and five
PE rejection controls pass again. Those unloaded images contain the four portable
core modules; they do not include the new platform validator or prove its full
native image closure. With the two hashes above, the final local wrapper emits
twenty-three named artifact comparisons. Hash agreement is scoped reproducibility,
not legal originality, cross-compiler identity or future OS-image reproducibility.

[Windows CI 37780598062 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37780598062)
at evidence source `8b09ef30109ba42867c678e5812eb60c2bddb563`. Observed completed
166-file guard, pinned tool setup, bootstrap twins and the full final wrapper.
Seven 43-check Clang result sets include bootstrap twins plus the five-config
matrix; four 28-check GCC sets pass. Debugger/sanitizer/core/link/rejection and
new preboot/endian checks pass. All twenty-three named local/remote hashes match.
Remote tests generated their own synthetic inputs; no new operator keyboard
session or native boot occurred. This result update changes documentation only.

No firmware, UEFI ABI/real exit/entry/stack/exceptions, device or boot is tested.
Firmware eligibility and the proposed VM remain needs-review; all OS edition
gates and historical identity/parity stay open. Human provenance review pending.
