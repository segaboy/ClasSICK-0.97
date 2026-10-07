# TEST-0005 — SPEC-0002 Windows presentation

Date: 2026-10-07. Independent hosted adapter verification; Codex self-review,
human review pending. [SPEC-0002](../../docs/specifications/SPEC-0002-windows-presentation.md)
and [IMPL-0003](IMPL-0003-windows-presentation.md).

## Fixtures and results

- Original literal 3x2 RGBA vectors include zero/partial alpha and distinct rows.
  Literal 9x2 mono vectors cover odd-width bit padding. Compare destination bytes,
  row/trailing guards, unaligned buffers and complete source preservation.
- Bounds: null/invalid metadata, adapter cap, stride/storage, SIZE_MAX/PTRDIFF_MAX
  overflow and error precedence. Literal viewport expectations cover crop,
  centering, zero/negative/INT32_MAX clients and output preservation on rejection.
- Actual GDI: eight DIB/DC cases inspect every RGB pixel at 1x/2x/3x and cropped
  sizes, with inset, background-only and empty clips. Independent literal colors/
  bits and supplied expected scale/origin determine results. GdiFlush precedes
  direct reads. Verify untouched outside-clip bytes and restored DC properties.
- Hidden native window: exact 1x/2x physical client sizes, create/resize/print,
  DPI context, Space switch/repeat suppression and WM_CLOSE destruction. CTest
  imposes a 15-second timeout. Native key scaffolding is not a core event queue.

Pinned LLVM-MinGW 20260908/Clang 23.1.1, CMake 4.4.4, Ninja 1.13.2; built-in Windows
PowerShell 5.1. Final local Verify-Presentation.ps1 -Sanitizers run passed:

| Configuration/check | Result |
| --- | --- |
| x64 Debug, two fresh builds | 10/10 checks each; viewer and both test executables match |
| x64 Release / NDEBUG | 10/10 |
| i686 Windows execution on x64 | 10/10 |
| x64 ASan + UBSan | 10/10 after intentional detection probes validate both sanitizers |
| Strict warnings and optimized core import/data audit | Pass; no Windows import enters core |
| ARM64 little/big-endian core | Compile/import audit only; pass, no execution claim |

The ten checks are the prior six plus four presentation suites. Existing surface
matrix still runs 8,424 whole-buffer cases per configuration. Instrumentation
covers core, adapter, tests and viewer; the freestanding object is uninstrumented.
Exact source revision/artifact hashes are pinned by the subsequent build snapshot.

## Development failures and limits

Strict compilation initially rejected a signed GDI sentinel comparison, ANSI cursor
resource passed to a wide API, and absent modern-Windows feature definitions.
All corrected before verification. An expanded empty-clip fixture assumed a
zero-size IntersectClipRect returned NULLREGION; locally that assertion failed
before presentation. The fixture now selects an explicit empty region and checks
GetClipBox. Empty/background-only presentation regression cases pass.

A visible earlier viewer build was launched; its original client was captured
locally through PrintWindow and visually inspected by Codex for sharp 2x geometry
and channel colors. The final refinement adds clip handling without scene changes.
This is render review, not owner manual acceptance or monitor color measurement.
No capture/binary is tracked in Git. Remote CI is recorded only after completion.

Not verified: real cross-monitor DPI change, physical colors, host-failure injection,
performance, older Windows, other host OSs/compilers, ARM64/m68k execution,
interactive debugger, arenas/normalized events/clock, B1, any OS boot, historical
behavior or edition parity. No QuickDraw/desktop equivalence follows from this scene.
