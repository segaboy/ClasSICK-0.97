# Current-core link and second-compiler evidence

Date 2026-10-08. ADR-0009 / IMPL-0009 / TEST-0011. Existing surface/arena/input/
clock contracts are unchanged. This checkpoint proves a complete standalone link
for the current bounded profile and exercises another compiler. It does not boot
or execute any standalone image. B1/M0.1 remains passed through its separate audit.

The original integration body runs under hosted Clang/GCC with guarded caller
storage. All four core modules plus that body link without default startup or
libraries under Clang/LLD 23.1.1 and GCC 16.2.0 / GNU ld 2.47.20260726, at O0/O2
for x86-64/i686. Complete maps retain eighteen original functions. The auditor
requires the actual entry point to match our named function, no runtime libraries,
zero imports/TLS/IAT/delay dependencies and valid bounded PE fields. GNU ld's
empty import terminator (24 bytes on x64, 20 on i686) is explicitly checked; LLVM independently reads
imports. These native-subsystem PE files are unloaded link fixtures, with no
firmware entry ABI, kernel stack, handoff or driver initialization.

Full verification protocol: 37 Clang checks per five configurations, validated
sanitizers, core/endian audits, six x64 debugger sessions/three rejection launches;
22 GCC core checks per Debug twin/Release/actual i686. Five GCC hosted executable
hashes and eight pairs of complete link-image hashes must match. Four unresolved-
helper failures and five image rejections test failure detection, including an
actual imported-DLL image. Detailed development failures are retained in TEST-0011.
The final fresh local run passes that entire protocol. Fresh offline tool setup
also passes from the pinned cache, including archive extraction and executable
hash validation. Immutable source/hash and remote pointers are recorded below
after publication; no remote pass is presumed.

## Repeat

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Setup-Windows.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Setup-SecondCompiler.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-CoreLink.ps1 -BuildRoot C:\ClasSICK\core-link-local -Sanitizers
```

Use a fresh directory. Tools/cache/binaries/maps/logs/JSON remain ignored/local.
Setup preserves incomplete directories instead of deleting them. The optional
GCC package is hash-pinned, extracted by the Windows archive reader and never
launched as an interactive environment. Existing CMake/Ninja remain in use.

GCC's broader Windows viewer build is unverified after a header conversion warning;
only portable core conformance is claimed. Hosted test executables use external
startup/runtime for testing; no test artifact is a product release. Standalone
profiles currently require no runtime helpers. Revalidate if code, flags, compiler,
target or runtime changes; review any future helper implementation separately.
ARM64 remains compile-only under Clang; m68k/128K and all OS edition boots remain open.

Next: review exact VirtualBox/firmware/device choices and specify the UEFI loader,
memory-map/handoff and post-handoff runtime before B2 implementation. The real
Macintosh/mini vMac independent-firmware goal remains a distinct native target.

## Immutable local result

Compiled/tested code and build scripts: `48aebc72aa10b2575c57bb85483c1ca7e211efa2`.
Evidence publication changes documentation only. Local Windows PowerShell 5.1
fresh final matrix and cached extraction replay both exit zero. Earlier failed
wrapper runs and exploratory wrong-entry links remain distinct in TEST-0011.
The public repository guard passes all 156 indexed text/source files. Codex
self-review; human provenance review remains pending.

Clang/LLD/LLDB 23.1.1, CMake 4.4.4, Ninja 1.13.2 retain existing package pins.
GCC 16.2.0 / GNU ld 2.47.20260726 use w64devkit 2.10.0: archive SHA-256
`18d0a4c71a166f8401ab6305781bec5882b40b5e06ba9807c61cb5f3b3c6325e`;
compiler `9a52de3e3af8f4896e963c4b68ed159f049954775decb20e421cd75d51aab6ed`;
linker `354e557b21c3c0c73da2be7e42891eb0298cdf455a54f6dbbc7c6d9b53a1e349`.
The stock Windows archive reader was bsdtar/libarchive 3.8.8. No distribution
launcher was run or new system configuration adopted.

Each row's independent fresh image twins match. All have eighteen retained
functions, exact declared entry, zero real imports and zero runtime libraries.
Cross-compiler or cross-profile hashes are not expected to match.

| Profile | Image bytes | Empty import bytes | SHA-256 |
| --- | --- | --- | --- |
| clang-64-O0 | 10240 | 0 | `81817cc1cc6166f9079dff32bc10a4165103dc16e6e7c20eb88676620f5ff9e7` |
| clang-64-O2 | 7168 | 0 | `010087001bb651974bb6a6dfce7de2023f92163f608d80df0c1b6278cb66388e` |
| clang-32-O0 | 9216 | 0 | `b11dd0b9f4ab112e93a2f6cfb011242a880d4845a847f22e629c39db56d1175a` |
| clang-32-O2 | 6656 | 0 | `91e4cf625774f4ee69cbf10e343691aaefcb015a62c83af083a65772afc74e38` |
| gcc-64-O0 | 11533 | 24 | `76cd8fe78ac9ab8c65321fe3457d8642bfa42f7f30be37a2c867783b040901a7` |
| gcc-64-O2 | 9796 | 24 | `42f74786aaa4b7613869d4af2ffe3811f9c7e08655f069ecf1b9af72b2d6f5a5` |
| gcc-32-O0 | 11114 | 20 | `4651bf7400772a992b43d414213e9a5e5068c1718c8af6e861811dd14af5fd5d` |
| gcc-32-O2 | 10903 | 20 | `eaf02fed90fbcbe0cc9db939df27a8ea2c9be57279135c9316df7730289ed1b7` |

GCC Debug-twin hosted hashes also match:

| Executable | SHA-256 |
| --- | --- |
| classick_surface_tests.exe | `0b884aa722909866278c20b4772a20ac7ff821c6cdb6eba3f6c8e298f9b46ba9` |
| classick_arena_tests.exe | `0a0cca4167d5b63f55c1e1c409522a428127191a2e06a7e538d681ad6784b4ab` |
| classick_input_tests.exe | `acdace625ac8c5d48fb3cc346826a3f99c4cae6ea3b5c05f95bd532aa1ad573f` |
| classick_clock_tests.exe | `b1a77e033faed2270c38799f391d87946e6708cc952d2333388f4587f83241a4` |
| classick_core_link_tests.exe | `03e59cff82dde5f5667c6d4c497ffa27dcdbbf6562314319b65c9afda281ee71` |

## Observed remote verification

[Windows CI 37774811569 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37774811569)
at evidence source `3950f3304dc21b1d611085542dca24939ca53b09`. Completed logs
show the 156-file guard, fresh pinned Clang/GCC setup, two bootstrap builds,
full five-configuration 37-check Clang matrix, validated sanitizers/core/endian
audits, six x64 debugger sessions/three argument rejections, four-configuration
22-check GCC matrix, all sixteen standalone links and nine failure controls.
All twenty-one named local/remote hashes match: eight existing Clang hosted
executables, five GCC hosted executables and eight standalone profile images.
The preceding local-results publication did not predeclare a remote pass.
This result update changes documentation only; compiled/tested inputs are unchanged.
The prior B1 operator result remains separate; no new keyboard session is claimed.
