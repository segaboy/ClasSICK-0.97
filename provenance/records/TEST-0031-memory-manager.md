# TEST-0031 - Bounded Memory Manager verification

2026-10-12; SPEC-0023 / IMPL-0027. Local verification passed.

Independent live-interval and byte-guard oracles cover pointer/handle creation,
logical sizes/even extents, lifetime/slot exhaustion, zone changes/limits, lock
idempotence/direct flag writes, resize/compaction/content preservation, failed
request atomicity, forged/empty/foreign cells, copy overlap and register results.
Test errors and malformed state separately from routine result codes. Original
common C/register probe and machine-readable interface validation; no x86
application or guest reference executable. Compiler width/endian/sanitizer and
standalone 68000 linkage are distinct from native execution/full heap budgets.

The full local chain passes 1,245 CTest checks in nine configurations:
151/151/151/124/151/136/136/136/109. Twelve behavior suites and three object
audits are added per configuration. The model suite exercises 1,500 deterministic
mixed operations and checks every surviving block/pattern/lock after each step;
65,448 unsupported register-boundary words preserve the frame. The common probe
passes 20 checks/status 0/signature 48454131 across all eleven routines. Thirteen
interface records and eight refusal controls pass. Thirty extra manager object
audits pass across Clang/GCC O0/O2 32/64-bit and ARM64 little/big-endian;
both manager test-executable debug twins match. No new runtime imports/globals.
Log SHA-256 bb728fe7e23ddd0ac386f9e567cc9768eb97753a03e0ff89e793b2821a364d3d.

GNU 68000 GCC 13.3 O0/Os passes ten object audits and two complete relocatable
links. Metadata 672 bytes, native record 20 and guest cells 256; maximum reported
function frames 608/604, not measured stack high-water or final linked-ROM/RAM
budget. Final fingerprints include the spec, registry, scripts and tool lock.
Six existing EFI profiles match the prior checkpoint; every old ROM input is
byte-identical to its prior admitted baseline. No VM/emulator/boot-entry change.
Native and historical claims are not extended from these results. CI comparison
is a separate publication follow-up.

The PDF renderer first lacked an optional package; the bundled PDFium renderer
completed the selected-page review. Original draft notes stay unchanged and are
versioned as research, with SPEC-0023 taking precedence over their proposals.
The first helper import created one local Python bytecode cache; it was removed
and bytecode writes disabled. No cache/binary/reference page is staged. During
implementation review, native-pointer offset expressions were parenthesized
before compilation so arithmetic uses checked offsets; all code tests passed.

Publication follow-up (2026-10-12): compiled revision
`58a1ca5ef680e2e7f7eed85d3880bbed42eb675f` passes both jobs in
[CI38328896626](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38328896626).
The complete manager manifest matches local exactly, including every input,
compiled fingerprint, function-frame list and storage budget. CI rebuilds the
existing own ROM twins/controls; both profile hashes and all ROM inputs match
the prior local baseline. Six existing EFI profiles also agree. No new native
execution is inferred. [Sanitized evidence](../../docs/development/memory-manager-evidence.md)
and JSON publish the bounded result and full fingerprints.
