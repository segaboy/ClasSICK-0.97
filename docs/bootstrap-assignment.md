# Founding assignment coverage

Date: 2026-10-07. This index maps the owner's 30 requested bootstrap items to
reviewable artifacts. It does not convert future architecture plans into achieved
OS behavior. Final publication/CI results are tracked in the evidence/status records.

| Item | Result / authoritative record |
| --- | --- |
| 1 Public repository | [segaboy/ClasSICK-0.97](https://github.com/segaboy/ClasSICK-0.97), public visibility verified |
| 2 Concrete slug | `ClasSICK-0.97`; human-facing name preserved in README/charter |
| 3 Initial structure | Core/personality/cpu/arch/platform/formats/tools/applications/tests/docs/provenance/scripts ownership notes |
| 4 README | [Project overview](../README.md) |
| 5 Clean-room policy first | [Binding policy](clean-room/POLICY.md); first public commit `d3f3af8` |
| 6 Provenance rules | [Ledger rules](../provenance/README.md) and templates/source catalog |
| 7 ADR system | [ADR index/template](adr/README.md) and eight initial decisions |
| 8 Environment inventory | [Sanitized snapshot](development/environment-inventory.md), reproducible probe script |
| 9 Missing tools | Inventory separates verified tools, aliases and not-found probes |
| 10 Minimal toolchain | [Evaluation](development/toolchain-evaluation.md) and pinned LLVM-MinGW/CMake/Ninja |
| 11 Explicit tool purposes | [Setup procedure](development/windows.md) and version/hash lock; local-only preparation |
| 12 Charter and success | [Charter](charter.md) with bootstrap/hosted/native/release gates |
| 13 System architecture | [Overview](architecture/overview.md) and import/dependency constraints |
| 14 HAL strategy | [HAL ownership/contracts](architecture/hal.md) |
| 15 System 1 architecture | [Personality managers/ABI](architecture/system1.md) |
| 16 68000 application strategy | [Contained guest execution](architecture/legacy-m68k.md) |
| 17 Hosted Windows strategy | [Hosted adapter](architecture/hosted-windows.md), build/setup/evidence |
| 18 Initial targets | [Target matrix](architecture/targets.md), explicit planned/verified distinction |
| 19 Language/build choices | Toolchain evaluation; ADR-0003/0004 remain revisable |
| 20 Testing/compatibility | [Methodology](testing/methodology.md), profiles and matrix |
| 21 Documentation/provenance | Stable IDs, claim classification, templates and source catalog |
| 22 Milestone roadmap | [Dependency-driven roadmap](roadmap.md) |
| 23 Unknowns/research | [Research and debt register](research/questions.md) |
| 24 First subsystem | [SPEC-0001 bounded surfaces](specifications/SPEC-0001-surfaces.md), no implementation yet |
| 25 Boot dependencies | [Mermaid dependency graph](architecture/boot.md) |
| 26 First successful boot | B0/B1/B2 exact gates; B2 requires post-firmware-handoff ownership/input/progress |
| 27 Immediate ADRs | Eight initial ADRs; future JIT/target/runtime decisions require new records |
| 28 Future specialist chats | [Proposed responsibilities/handoffs](governance.md); none created |
| 29 License options only | [Comparison](development/licensing.md); owner approval required, no license adopted |
| 30 Public bootstrap commits | Founding safeguards already public; completed documentation/tooling publication recorded in status/evidence |
