# Project HQ and future specialist responsibilities

The owner has final scope/licensing authority. This founding chat is Project HQ;
the public repository is authoritative. No specialist chats are created during
bootstrap. The following is a proposed division when the owner requests them.

| Role | Owns | Deliverable to HQ |
| --- | --- | --- |
| Reference/specification | Source eligibility, version baseline, authorized black-box experiments | Behavior specs and original probes/results; no internal Apple algorithms |
| Portable core | Arenas, surfaces, events, devices, VFS contracts | Native/freestanding code and contract tests with import audits |
| System 1 personality | Trap ABI and manager behavior from approved specs | Spec-to-test-to-implementation trace and compatibility matrix |
| Storage formats | MFS and resource-fork codecs, synthetic generators | Bounded parsers, corruption tests, safe interoperability reports |
| x86-64 boot/platform | UEFI handoff and own post-handoff devices | Reproducible B2 image, machine config and evidence |
| Macintosh hardware / m68k native | Published hardware/ISA research, independent ROM/startup, size budgets | Cold-start gate evidence and native link/runtime proof |
| Legacy 68000 runtime | Guest memory, ISA interpreter, callbacks/trap bridge | Original instruction/ABI probes; no whole-machine dependency |
| Tooling/QA/provenance | Setup, CI, checks, fuzzing, source/fixture review | Repeatable evidence and policy-compliant publication controls |

Start with core and specification roles; avoid creating all roles before there is
work to review. Conceptual separation applies with one contributor. Separate
reference and implementation personnel later if the owner adopts that stronger
process; do not retroactively claim independently staffed clean-room teams.

## Handoff contract

Each task names its files, scope, governing ADRs, approved inputs, expected tests,
profile, and definition of done. Reference tasks publish specs before dependent
implementation tasks start. Every implementation handoff includes source/spec/test
IDs, provenance disclosure, results, limitations, and unresolved questions.

HQ resolves interface ownership and architecture conflicts through ADRs. Changes
to a shared contract are reviewed before dependent teams implement them. Branches
must pass clean-room review before public publication; private working branches
are not permission to derive code from prohibited sources. Never pass prohibited
material through a chat handoff or AI prompt.

Update status and debt records after each accepted milestone. Use explicit evidence
for achieved claims and distinguish experiments/plans from validated behavior.
