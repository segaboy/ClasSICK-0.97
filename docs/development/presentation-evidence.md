# Windows presentation verification snapshot

Date: 2026-10-07. Executed implementation/specification/test/build inputs:
`bb2a9f39205175cb6cf4c88f692c4fd9feea29a9`. This later text record adds immutable
pointers; it does not change the verified code. SPEC-0002 v1 / IMPL-0003 / TEST-0005.

Windows 11 x64, built-in PowerShell 5.1, pinned LLVM-MinGW 20260908 / Clang 23.1.1,
CMake 4.4.4, Ninja 1.13.2. The final fresh Verify-Presentation.ps1 -Sanitizers run
passed all ten CTest checks in each of x64 debug twins, Release, i686 and x64
ASan/UBSan. Existing independent sanitizer detection probes passed. ARM64 endian
core compilation/import audits passed; no ARM64 execution. Core bytes unchanged.

Fresh x64 debug executable SHA-256 values:

| Artifact | SHA-256 |
| --- | --- |
| classick_surface_demo.exe | be374d0005643fe03c38d53d37f4c4e7639bce7f6c319623a087958dac0b521e |
| classick_presentation_tests.exe | 4cde2257f71a4ceec66fb6871a36ca5a5d452e342292a6c567eee8e06d35f01d |
| classick_surface_tests.exe | 6c44fa17d22f384f7a9e1837f887f39658df5adf30948fdc23a33b7ce4bcc667 |

Hashes matched two fresh local output directories. Static archives/OS images,
other compilers and all machines are not covered by this repeatability claim.
Four new checks cover conversion, bounds, eight exact RGB GDI render cases and
hidden-window create/resize/print/DPI-context/switch/close. The existing core matrix
still covers 8,424 whole-buffer cases per configuration. Runtime/pixel tests are
independent contract fixtures, not historical reference observations.

The original color scene was captured from an earlier visible preview via
PrintWindow and visually inspected by Codex; final source adds empty/background-only
clip refinements without changing artwork. The final viewer is available for owner
manual review. No binary/image is tracked or published as a release artifact.
The verification wrapper and manual run/review steps are in
[Windows development](windows.md); detailed fixtures/failures/limits in
[TEST-0005](../../provenance/records/TEST-0005-windows-presentation.md).

## Remote validation

[Windows CI run 37691938711 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37691938711)
at `5c166dddc15069329aec153e2c707d1e9f61a1f0`: public 100-file guard, pinned setup,
bootstrap twins and the full presentation/surface matrix, including both sanitizer
detection controls and instrumented tests. All ten CTest checks pass per
configuration. CI viewer/presentation/core test hashes equal the local values
above. This CI revision differs from executed implementation bb2a9f3 only by text
evidence/status; code, tests, build scripts and contract are unchanged.

No B1/M0.1 closure,
native boot, historical behavior, Linux/original-hardware edition or parity claim.
Arenas, normalized events, clock/debug workflow and actual cross-monitor DPI tests
remain open. Self-review is complete; separate human review remains pending.
