# Tests

`uefi-loader.c` (TEST-0013) exercises original x64 EFIAPI callbacks, independent
CRC and finite exit traces, malformed data/ownership/cleanup and independent bundles.
It uses live allocated mock storage and never executes privileged transition code
or accesses a physical framebuffer. Native EFI inspection/failure controls are
separate; see [loader evidence](../docs/development/uefi-loader-evidence.md).

Reserved for core/subsystem, API, trap, format, graphics, portability and native
boot suites using independently authored inputs and rights-cleared fixtures.
No historical behavior tests are implemented yet. `surfaces.c` is TEST-0004:
independent literal bytes and 8,424 whole-buffer cases from SPEC-0001, split into
layout, errors, matrix and independent-surface CTest suites. It uses explicit
failure returns, including release builds with NDEBUG. `surfaces.freestanding`
audits an uninstrumented optimized core object for imports and global data.
`sanitizer-probe.c` intentionally contains defects and is run only by explicit
sanitizer validation, never as a normal conformance test. Bootstrap CTest still
runs the development probe from `tools/toolchain-smoke/`.
See [results](../provenance/records/TEST-0004-surfaces.md).
See [test strategy](../docs/testing/methodology.md).

`arenas.c` is TEST-0006: literal aligned layouts, stable errors/transactional
rejection, reset/reuse, independent ownership and an offset-scanning oracle.
The tested x64/i686 alignment limit is 16, producing 751,740 whole-buffer matrix
cases per execution. Typed native allocation is exercised over host-allocated
storage only. `arenas.freestanding` audits the optimized core object, while
`arenas.viewer-memory` exercises the real viewer with a one-byte-short pool.
The ordinary hidden-window test also checks exact successful buffer reservations.

`core-link.c` is TEST-0011's hosted integration harness: independently guarded
allocated storage, rejected capacities, separate ownership and the original
four-module probe in `tools/core-link/`. `freestanding.probe` runs this behavior
under Clang and GCC. The separate no-startup/no-library link and rejection matrix
is run by `Verify-CoreLink.ps1`; its PE files are never loaded or counted as boots.
See [scope and repeat instructions](../docs/development/core-link-evidence.md).

`uefi-contract.c` (TEST-0012) uses 11,520 framebuffer cases, 10,240 bitmap-oracle
map layouts, maximum map/allocation limits, ownership/error guards and 27 finite
exit-outcome traces. Five behavioral suites and one optimized import/data audit
raise the current matrix to 43 Clang / 28 scoped GCC checks per configuration.
Physical addresses are metadata only; no firmware is invoked. See
[the preboot snapshot](../docs/development/uefi-contract-evidence.md).

`native-framebuffer.c`, `boot-scene.c` and `native-present.c` (TEST-0015) cover
SPEC-0009. They compare 21,600 guarded placement cases against a per-pixel
expectation, and the scene against the independent point classifier in
`scene-oracle.h`. They also check band-partition independence, staging budgets
and arena exhaustion, the exit/ready gates and the 32-byte trace record. Only
synthetic buffers are written; no firmware or device is touched.

`acpi.c`, `pmtimer.c` and `native-timer.c` (TEST-0016) use original synthetic ACPI
images from `acpi-fixture.h`, written from published field offsets. They cover
every walk result, reader refusal at each stage, wrap extension against summed
steps, time conversion against a native-division oracle, the map-type reader
rules and probe traces. The loader `configuration` suite covers RSDP GUID
selection and bounds.

`native-loop.c` (TEST-0017) drives synthetic counters through the real presenter
and scene. It covers gates, 24/32-bit wrapping progress over 1–60 seconds with
exact frame and read counts, late samples, a 20,000,000-read stall and
out-of-width values. The final frame is compared with the scene oracle.

`ps2.c`, `acpi-keyboard.c` and `native-keyboard.c` (TEST-0018) add original
controller transcripts and scan streams, positive FADT controller gating,
two-key FIFO behavior, bounded startup/retry/drain failures and interactive scene
oracles. `ps2-fixture.h` supplies callbacks without touching hardware. Ten new
x64 / six portable checks include optimized import/data audits; real keyboard
events, controller timing and boot acceptance remain unobserved.
