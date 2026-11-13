# TEST-0165 - 286 span-construction boot diagnostic

2026-11-13; SPEC-0082 / IMPL-0118. Two pinned 16-bit builds and BootHush runs
matched. An actual split bank and nonzero-offset far-pointer conversion reached
POST `A8`; the wrong boundary expectation reached `B5`. Complete payload and
both physical windows matched at each stop, media was unchanged, and near-data
compilation was refused at the pointer-width assertion. This is a bounded
emulator/model result, not a production provider or OS boot. [Evidence and
limits](../../docs/development/boothush-developer-report-20261113.md).

Later 2026-11-13: Twin 16-bit runs matched while executing planner, binder and
checked read/write together; expected refusal controls reached `B5`, `B7` and
`B8`. [New evidence](../../docs/development/boothush-developer-report-20261113-b.md).
