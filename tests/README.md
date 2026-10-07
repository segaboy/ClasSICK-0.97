# Tests

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
