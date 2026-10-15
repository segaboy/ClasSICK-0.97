# TEST-0041 - Original data-only owned CODE image conformance

SPEC-0033 / IMPL-0037 / SRC-0178; 2026-10-15.

Original tests/code-image.c reuses the project's own declarative resource encoder
with independently asserted guest bytes, ownership, world and failure-state oracles.
No manual samples, historical application/resources/assets or raw research fixtures.
No generated CODE bytes execute. Nine production suites, one test-only fault suite
and one production import audit run across nine compiler/width/optimization/
sanitizer profiles. Test injection symbols are absent from production objects.

success, capacity, ownership, cancel, aliases, seal, resources, profile and model:
freed source before LOAD (ASan too); exact bytes/odd padding/shared targets/full
world/parameter/slack/boundaries;136 segment partitions;64/16/max524816 staging on
an explicitly larger hosted map; actual locked-fragmentation first/later OOM;
shared32 slots, mixed system/application objects and resident scrap; real retained
compaction after later OOM/partial release; unsafe native protection and original/
changed map backings; physical EXEC/heap/GUEST/native aliases; generation saturation,
changed epoch/input/staging; no-effect ownership/cell/world refusal; native-only
CANCEL with independently faulted life/changed manager/map/epoch; associated cache
and independently constructed inactive session gates. No full128K implication.

Fault conformance asserts independent135 LOAD and85 RELEASE semantic units, injects
after every one, checks exact acquired/locked/copied/live/disposed/retired masks,
world/table byte progress, canonical output/low/system/stack preservation and first
fault immutability. Six before-manager-call failures and one pre-publication
PREPARE refusal separately test attempted/completed and validity tags. Ordinary
real -108 is not conflated with test-only unexpected-failure seams. FAULTED refuses
all5 later operations, creates no fresh data authority and blocks external admission.

[Results](../../docs/development/code-image-evidence.md) distinguish actual final
checks from worker suggestions and pre-final failed attempts. No historical,
physical/cumulative-stack/full128K, boot, native-application or B2 promotion.
