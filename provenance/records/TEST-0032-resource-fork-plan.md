# TEST-0032 - Resource-fork conformance plan

2026-10-13; SPEC-0024 / ADR-0028 / IMPL-0031 / SRC-0106/0126/0127.
The owner authorized execution after the original intake. Current coverage is
recorded here; accepted counts/revisions belong in separate development evidence.
The original planning section below retains its earlier scope.

## Implemented conformance

Five hosted suites, an independent-model suite and a freestanding audit.
Fourteen small declarative variants include empty types/resources, signed-ID
endpoints, nonprinting/zero type keys, no/empty/255-byte names, odd positions,
partial prefix, gaps, trailing bytes and map-before-data. Every truncated length
of those variants is tested in an exact-sized native allocation; removing only
tolerated trailing bytes can remain valid. Additional raw-prefix overlap and
unaligned input cases check L-12 and bytewise decoding.

Twenty-seven explicit malformed cases check header/region carry and bounds,
fixed/table/reference overlaps/order, bad names, local length-prefix/payload
limits and structural-before-content precedence. Argument sentinels, failed
reconstruction of a still-valid view and interleaved independent views check
publication and lifetime rules without deliberately dereferencing invalid native
pointers. Requested duplicates preserve entries and unrelated keys remain usable.
Shared local/name spans and warning combinations are accepted; references with
misleading FFFFFF data offsets expose only their opaque specification.

Large generated cases exercise unsigned type/reference/name positions at8000,
the maximum24-bit length-prefix offset with a data region beyond16MiB, payload
length32769 and a5000-entry dense list. Their semantic boundary truncations are
checked; large payload bytes are not redundantly swept at every length. Large
host buffers do not claim a128K execution profile.

The original444-byte six-resource fixture has113664 single-byte mutations.
An independent Python arbitrary-precision range model reconstructs semantic
records and determines exact open status/field/position/warnings/work plus lookup
status/metadata. The C harness compares all cases, output refusal atomicity and
unchanged input. Python and C encoders independently agree on the base bytes.
All generators/model/harness are delivered; the earlier undelivered checker is
not used. Numerical outcomes and superseded preparation failures are recorded
after the final compiler/CI checks.

## Original intake plan

Initially planned before implementation, 2026-10-13; SPEC-0024 / ADR-0028 /
SRC-0106. At that intake there was no parser code or executed conformance.
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

Accepted exact revisions, outcomes and limits:
[reader evidence](../../docs/development/resource-reader-evidence.md).
