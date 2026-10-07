# TEST-0004 — SPEC-0001 v1 headless conformance

Date: 2026-10-07. Classification: project contract verification, not historical.
Author/reviewer: Codex, self-review; separate human review pending. Implementation:
[IMPL-0002](IMPL-0002-surfaces.md). Contract:
[SPEC-0001 v1](../../docs/specifications/SPEC-0001-surfaces.md).

## Independent fixtures and method

`tests/surfaces.c` contains original literal byte vectors for MSB bit boundaries,
RGBA byte/channel order and clear/padding. The matrix oracle inspects every storage
bit/channel and tests coordinate membership against the input rectangle, without
implementation clipping or helpers. Each operation compares the entire 2,048-byte
test array, including guards, untouched pixels, row/bit padding and trailing bytes.

Matrix: 2 formats × 9 widths (1, 7, 8, 9, 15, 16, 17, 31, 65) × 3 heights
(1, 3, 7) × 4 padding sizes (0–3) × (19 rectangles × 2 values + clear) =
**8,424 whole-buffer cases**. Rectangles cover all edges, exact bottom-right pixel,
empty/disjoint/reversed inputs and INT32_MIN/INT32_MAX. Focused suites cover stable
error values/precedence, null/invalid metadata, zero/negative dimensions, row/span
overflow, PTRDIFF_MAX bound, exact full-row storage threshold, transactional
construction, unaligned byte storage and interleaved independent surfaces.
Failure returns survive NDEBUG; no assertion or image capture is the oracle.

## Executed results

Existing pinned LLVM-MinGW 20260908 / Clang 23.1.1, CMake 4.4.4, Ninja 1.13.2,
Windows 11 x64. The wrapper was exercised with built-in Windows PowerShell 5.1.
No new tool download, reference payload or system configuration was required.

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-Surfaces.ps1 -BuildRoot <fresh-output-directory> -Sanitizers
```

| Check | Result |
| --- | --- |
| x64 debug, two fresh builds | All 6 CTest checks pass in each; surface test executable SHA-256 matches |
| x64 release / NDEBUG | All 6 checks pass |
| i686 Windows executable on x64 Windows | All 6 checks pass, including actual 32-bit row overflow rejection |
| x64 ASan + UBSan | All 6 checks pass after separate intentional heap/signed-overflow probes fail with the expected diagnostics |
| Strict warnings | Wall/Wextra/Wpedantic/Werror; core/test Wconversion/Wsign-conversion pass |
| Optimized freestanding surface object | ffreestanding/no-builtin/no-stack-protector compile; no undefined symbols or global data, audited by llvm-nm |
| Debug core archive/header inspection | No undefined symbols or mutable data; only stddef/stdint dependencies |
| ARM64 little/big-endian ELF | Compile/import/data audit passes, no execution claim |
| Negative audit controls | Independent external-call and mutable-global probes each rejected with nonzero status |

The six CTest checks are one bootstrap smoke, four surface suites and one optimized
object audit. ASan/UBSan instrument both the library under test and test executable;
the freestanding object audit stays uninstrumented. Windows startup, console and
sanitizer imports in development executables do not belong to the portable core.
Exact immutable source/artifact hashes are recorded in the subsequent
[build evidence](../../docs/development/surface-evidence.md); this record is also
part of that versioned source snapshot.

## Limits and retained failures

An early verification run stopped on differing static archive hashes. Object-header
inspection identified varying COFF timestamps; the wrapper now records both hashes
and limits the reproducibility assertion to the executable. It does not promise
identical core archives, future OS images or results across compilers/machines.
An initial interactive sanitizer command was rejected by PowerShell's comma parsing;
the quoted flag and final wrapper run passed. No surface conformance defect was
observed. ASan does not check writes within a larger live array, which is why exact
whole-buffer/guard comparisons are required too.

Remote run 37684200832 at evidence revision 81a6565 passed the guard, setup,
bootstrap, x64 debug/release, i686 and ARM64 audits, then failed UBSan probe
signature validation before the sanitizer conformance build. Its probe diagnostic
was not emitted by that wrapper, so the log alone does not prove the cause.
The verification wrapper now captures native stderr directly to a file through
a hidden process, avoiding PowerShell error-record formatting and emitting a
bounded diagnostic on failure. The complete local matrix passed again, with the
same test executable hash. Core, conformance tests and v1 behavior are unchanged.
Remote sanitizer validation is recorded separately when its rerun completes.

The [corrected remote run 37685301136](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37685301136)
completed successfully at `aeb7b1d57182a2bc254bbcfe1c34cb3c390abad4`, including both
intentional detection probes and the full instrumented/headless matrix. Direct
stderr capture preserves signatures regardless of PowerShell formatting; an
independent local narrow-width control demonstrates that the previous formatting
can split the signature. The original remote failure's raw diagnostic is unavailable,
so its exact cause is not asserted beyond the observed validation failure.

Not run: m68k compilation/execution, ARM64 execution, second compiler, Linux/macOS
execution, hardware framebuffer, boot/firmware link, interactive debugger, historical
reference behavior, fuzzing or three-edition parity. Synthetic tests establish only
the named generic contract cases. Pointer lifetime and truth of caller-supplied
lengths remain preconditions; no hostile-application isolation claim is made.
