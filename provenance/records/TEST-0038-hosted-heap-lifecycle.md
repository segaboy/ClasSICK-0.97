# TEST-0038 - Hosted heap-lifecycle conformance

SPEC-0030 v2 / ADR-0034 / IMPL-0034 / SRC-0151..0154; 2026-10-14.
Original `tests/heap-lifecycle.c` generates its own synthetic RAM, allocation
patterns, context/descriptor mutations and owned-control sentinels. No manual
sample, Apple resource/application/disk, or research-plan fixture is copied.
Research and worker-proposed cases remain unexecuted as proposals.

Nine hosted suites cover initialization, admitted reset/teardown and repeated
scrap preservation, complete tagged matrix/live-zero versus NIL, exact physical
limit/slot/staging boundaries, epoch saturation, single-use/foreign/cancelled
plans, changed inputs, inventory/patch/context refusals, native/guest aliases,
coordinated gates and numerical handle reuse. Byte-level comparisons preserve
system allocations, gateway, session/stack and saved CPU report. An original
test-only suite sweeps every counted post-write fault boundary for reset and
teardown; reports retain actual effects, first failure and published-handle tags.
Production has a separate freestanding import/global-data check and no injection.

[Executed results](../../docs/development/heap-lifecycle-evidence.md) give exact
local final/retained check counts, compiler profiles, CI revision, manifests,
twins and partial storage/frame accounting. Tests execute hosted core contracts,
not native applications or VM/emulator/reference observations. No new guest
program is executed in this slice. Existing whole-regression coverage is run
without inspecting or editing UART source/test bodies.
