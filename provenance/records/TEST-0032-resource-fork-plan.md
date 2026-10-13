# TEST-0032 - Resource-fork conformance plan

Planned, 2026-10-13; SPEC-0024 / ADR-0028 / SRC-0106. No code or test run.
Claude's original draft has 14 proposed base variants and 37 case rows. Its
unprovided throwaway checker is not reproducible project test evidence.

Generate original fixtures from declarative type/ID/attribute/name/data records,
with invented content and explicit field overrides. Do not copy manual examples
or any Apple file, resource, image or application. Emit bytes independently from
the parser and check intervals/results with a separate model.

Adopt coverage for empty/single/multiple types, signed-ID endpoints, case-distinct
and nonprinting type bytes, names of 0/255 bytes, odd data positions, partial
prefix layouts, gaps, trailing bytes, map-before-data, unsigned offsets above
8000 and 24-bit length-prefix reach. Large synthetic fixtures are generated at
test time rather than stored. A resource length extending beyond the prefix's
24-bit reach must not be rejected solely for making the data region larger.

Adapt draft expected outcomes to SPEC-0024: bit 7 returns REFERENCE metadata;
the numeric type key has no null-type-pointer argument case. Shared-name/data
cases check reader acceptance, not an unimplemented audit. Test absent and
available diagnostic prefix spans, including overlap with tolerated data/map.
Ordinary nonzero handle fields and changed/reserved flags produce warnings;
bit-7 specifications are not ordinary zero-handle violations.

Retain truncation, carry, structural overlaps, wrong offset bases, reference-list
ordering, malformed names, local prefix/payload overruns, requested duplicates
and error-precedence cases. Revalidate expected field/offset diagnostics against
the final structural-first check order instead of treating the unprovided draft
checker as authority.
Add duplicates in unrelated keys, references with misleading local offsets,
and invalid constructor/argument/output lifetime combinations under the native
caller contract. Failure preserves input/view/entry; ARGUMENT preserves report.

For each base fixture, sweep every truncated length. The suggested 444-byte
fixture's 256-value sweep contains 113664 cases; this is a planned count only.
Check memory bounds, stable errors, validated spans, no mutation and bounded
entry reads. Interleave distinct views and exercise argument/output sentinels.
Do not use a checker sharing parser helpers as the sole oracle.

When implementation is authorized, use the existing scoped Clang/GCC width,
sanitizer and endian checks and standalone GNU68000 helper/frame audits. Record
actual results and budgets then. No VM/emulator or reference run is part of this
plan; no application executable, native SDK or boot entry is generated.

Large whole-buffer fixtures run in the hosted environment. A 128K execution
profile uses a budget-fitting subset; shared semantics do not imply storing a
17 MiB fork in 128K RAM. Standalone compiler audits are separate from execution.
