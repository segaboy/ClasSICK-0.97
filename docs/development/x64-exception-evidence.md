# Owned x64 exception scaffold verification

Date 2026-10-08. SPEC-0008 / ADR-0012 / IMPL-0012 / TEST-0014.
Original descriptor serialization and native terminal assembly. No native execution.

Repeat with pinned tools and a fresh output directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-X64Exceptions.ps1 -BuildRoot C:\ClasSICK\x64-exception-local -Sanitizers
```

The wrapper retains complete prior hosted/core/debugger/sanitizer/link/endian
verification, adds serializer checks and Clang/GCC hosted twin hashes, and audits
four own EFI images including descriptor installation and every vector/fault path.
Compiled/test/script source: `974d7817cf8ef087dbdcbd16443b5ca60682d8ee`.
Fresh local `C:\ClasSICK\x64-exception-verification-20261008-a` plus adjacent
UTF-16 PowerShell log passes full wrapper success and final native exit zero under
Windows PowerShell 5.1. Clang x64 53 checks per Debug twin/Release/ASan+UBSan,
actual i686 48; scoped GCC x64 38 per Debug twin/Release, actual i686 33.
Prior debugger, sanitizer detection, endian/core/link controls pass.

Four EFI images form matching O0/O2 twins: 22 original named symbols, three DIR64
relocations, all 256 exact vector stubs, owned install/capture bytes and one bounded
writable/non-executable pointer slot. Twenty image corruption controls and real
omitted-transition/exception-object links reject. Original O2 payload copy matches.
Twenty-nine named local comparisons match; 23 earlier fingerprints are preserved.
The two hosted loader Debug fingerprints change with the added CPU-tail constants
in loader.h's layout enum; loader orchestration and existing fixtures are unchanged.

| Artifact/profile | Matching SHA-256 |
| --- | --- |
| Clang hosted x64 table Debug | `d2c64925847d05c4cfafdb186fdf5e365c2cb569c8f62e386f3a55a2ceb6e0c8` |
| GCC hosted x64 table Debug | `35a5683d1e6319b1fb45cc5cbd3b9adf4c57440aee76ef013f158b811eed570d` |
| Updated Clang hosted loader Debug | `5fd9f6dbee17ab288b9df2363985c4282b44b2f48051fa24d7d0bc16e8d1605c` |
| Updated GCC hosted loader Debug | `0bb9542171fae557ea6bffd49f915cedc92fa502a8f41f629a8d8676d034d29e` |
| Clang x64 EFI O0, 22,016-byte file | `aba3a2651d750734cad70f35d7479c14438ce9468f4bbb883dbcc5355a554e49` |
| Clang x64 EFI O2, 17,920-byte file | `e944113e278873f042db201a3506cfbef578887b11de9480428d0ed11fdba325` |

[Windows CI 37795563016 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37795563016)
at the same compiled/test source: 193-file guard, pinned tools, bootstrap twins and
full wrapper. Six 53-check Clang sets/one 48-check i686; three 38-check GCC sets/
one 33-check i686. All 29 named remote fingerprints equal local results. Image
audits/control rejections and earlier protocol checks pass. No native EFI execution.
This result update is text only; tested implementation bytes remain unchanged.

The image is an unloaded terminal scaffold. Real descriptor installation/fault
delivery, native devices/event loop, firmware eligibility, formatted media and B2
remain unverified. The original Macintosh/mini vMac and all physical edition gates
remain independent. No OneNote publication or external firmware adoption occurs.
