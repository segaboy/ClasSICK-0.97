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
