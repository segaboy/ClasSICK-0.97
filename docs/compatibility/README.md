# Compatibility status

Baseline candidate: January 1984 Macintosh System 1.0/System 0.97.
Exact reference profile remains unverified; see R-001 in the research backlog.

| Area | Status | Evidence |
| --- | --- | --- |
| Native OS | Not implemented | Architecture and boot gates only |
| QuickDraw/Toolbox managers | Not implemented | Planned responsibilities; no approved historical contracts |
| MFS and resource/data forks | Not implemented | Research and generic device strategy only |
| Macintosh 128K startup/replacement ROM | Not implemented | Hardware/budget research pending |
| 68000 legacy application execution | Not implemented | Contained-runtime design only |
| Windows toolchain | Bootstrap verification tracked separately | No compatibility implication |

Future entries identify spec/test IDs, reference edition/profile, exact source
revision, supported operations, error behavior, target, safe observation evidence,
and remaining deviations. Use **documented**, **observed**, **partial**,
**unsupported**, **unverified**, or **extension** consistently.

An extension has its own opt-in contract and cannot overwrite a historical test's
expected result. Independently drawn artwork and changed typography are reported
as appearance differences. A running application is recorded by version and tested
workflow; it does not imply support for all its features or related applications.
