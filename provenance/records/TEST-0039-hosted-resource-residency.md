# TEST-0039 - Hosted resource residency conformance

SPEC-0031 / ADR-0035 / IMPL-0035 / SRC-0159..0162; 2026-10-14.
Original `tests/resource-residency.c` builds synthetic forks, RAM, profiles,
allocations, tickets and context/mutation controls. No manual samples, protected
resource/application/image, research-plan fixtures or worker code are copied.

Eight hosted suites: success, limits, tickets, source, integrity, aliases,
lifecycle and model. They cover same-key reuse/distinct tickets, zero reservation,
33001-byte and exact65000-byte maximum resources, shared file spans/distinct
copies, warning/name/attribute metadata, ambiguity/reference/system-heap refusal,
16/32/32 capacity, final valid ID and saturation, compaction/movement versus
no-effect OOM, foreign/replayed/mutated tickets and reused numeric handles.
Snapshots preserve source/RAM/manager/lifecycle/session/gateway/service controls.
Changed backing-map alias safety, captured/current epoch conflicts, truthful
FUTURE refusal, close/reset/reopen and prepared-state refusal are checked.
The original2500-operation model compares a separate residency/lease-count model
and strictly increasing issued IDs; its seed and arithmetic are in the source.

A separately compiled fault suite sweeps every loading unit, every three-entry
close disposal/retirement/final-publication unit, open/release publication,
before-allocation/lock/second-disposal failures and post-lock record corruption.
Actual masks, copied prefixes, allocated/issued IDs, close survivors and immutable
first diagnostics remain; fresh later FAULT refusals publish no borrow or effects.
Production has a freestanding import/global-data test and no fault seam symbol.

[Evidence](../../docs/development/resource-residency-evidence.md) distinguishes
initial full regression, final affected repeats, compiler/CI and storage checks.
No valid2GB payload or full uint32 source-size fixture is claimed executed.
Truthful native inventory/source lifetime, content authenticity, cumulative stack,
full128K and historical behavior are not proved by these original hosted cases.
Prior whole regression runs without UART body inspection/editing. No new guest
program, native application, emulator/VM/reference or boot trial is executed.
