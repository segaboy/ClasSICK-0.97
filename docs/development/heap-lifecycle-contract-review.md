# Hosted heap-lifecycle contract review - 2026-10-14

SRC-0146..0148 / SPEC-0030 / ADR-0034. Lead self-review; human provenance
review pending. Contract-only completion of runtime prerequisite 3's design.
No implementation, fixture, TEST/IMPL/build record, registry import or boot change.

## Owner scope, inputs and exposure

The owner requested assessment of the installed Grok CLI and best deployment
across platforms, then separate Snow/VirtualBox screenshots, then "Thank you.
Now, move on to the next tasks." The lead's stated next task is the heap-lifecycle
contract with a bounded independent review. The owner explicitly approved
transmitting the specified public review/plan sections to xAI for that check.
An initial automatic approval rejection blocked the new launch until that answer;
the accepted retry followed normal approval. No bypass or extra delegation.

Use the earlier reviewed HL/RA/PG dispositions and existing SPEC-0022/0023/0028/
0029 contracts. No new manual, source, sample, private heap structure, executable
or reference observation is an implementation input. Earlier historical exposure
logs and exclusions remain in force; this is not fresh clean-room certification.
No UART implementation/test body, new manager code or historical fixture is read
or changed. The raw Grok assessment/review and all22 research originals/copies
remain outside Git; no HQ content is exported into the public repository.

The source baseline is public33e203f; SOURCE catalog entries retain full hashes
and roles. Private preservation copies/identity manifest:
`C:/ClasSICK/heap-lifecycle-contract-20261014-a`. Five concurrent new research
files are preserved but not intaken here. The seventeen earlier originals retain
their prior dispositions; none becomes implementation authority by proximity.

## Grok check and lead dispositions

Grok1.0.52's returned report: `grok-stream-response.md`, SHA256
`f35d7e05f69b062c51f3a39040d49144557d44b8f16ee2f6b874945e6b04b8e9`.
Runtime final metadata reports grok-4.7-build, two model turns and six read_file
calls restricted to the six requested ranges in three public documents; no
command, write, web/MCP, subagent, implementation or manual read. Grok did not
independently check HEAD or execute a test. Codex checked baseline identities.

| Finding | Lead disposition / concrete contract consequence |
| --- | --- |
| ExitToShell and stop snapshots must not imply teardown or immutable RAM | Adopt. SPEC-0030 separates explicit inactive host teardown, preserves the SPEC-0028 CPU stop record, permits later deliberate RAM changes and leaves A9F4 unchanged. The relevant wording is independently confirmed in SPEC-0029; it was outside Grok's narrow ranges. |
| Partial commit and population need truthful retained fault state | Adopt with changes. Unexpected invariant failure locks the coordinator FAULTED with retained effects/staging; no recovery operation in v1. Predictable failures belong before commit. Repeated teardown is not assumed safe after corruption. |
| Complete scrap matrix is missing | Adopt. The contract supplies an original typed host NONE/RESIDENT/DISK-or-UNKNOWN matrix; it does not guess a raw historical ScrapState mapping. Live zero-sized blocks, NIL cells, unavailable content, system-owned scrap and name/pointer lifetimes are distinct. |
| Continuous global/reference coherence and system scrub ownership are incomplete | Adopt with changes. v1 supports the existing manager views, refuses unsupported registrations and every required system-storage scrub, and preserves system objects. The31-name inventory remains a future coverage checklist; arbitrary guest execution is not certified. |
| Counter width, saturation and reused-address aliases need decisions | Adopt. Native uint32 epoch starts1, saturates before staging and never wraps; cell/payload reuse remains explicitly indistinguishable. Repeated teardown can recreate scrap and alter handles/globals. |

The row questions are not decisions. Reject any inference that DefltStack's
authority or epoch width must remain forever unselected: these are necessary
engineering choices here. Historical HL-Q2/14/17 remain open. Do not treat an
excerpt boundary reported as unavailable as a missing source file or unresolved
runtime fact. The check's output exceeded the requested1000-word target; accept
the bounded technical findings, not a claim of strict output-limit enforcement.

## Requirement traceability

| Reviewed proposal | SPEC-0030 disposition in the first profile |
| --- | --- |
| HL-E1 phases | Exact preflight/staging/revalidation; first destructive epoch/ownership boundary; reset population and teardown SCRAP_PRESERVE are explicitly distinct. |
| HL-E2 refusal | Protected state unchanged before commit; only owned staging/plan/report may change, with aliasing/unwritable report refusal. |
| HL-E3 retained fault | Partial state and first cause retained, no rollback/repair/session entry; FAULTED blocks subsequent coordinated use. |
| HL-E4 teardown | Host-only/inactive; no automatic fault/ExitToShell teardown or goodBye; repeated teardown may change handles/cells/limit. |
| HL-E5 scrap | Full typed host matrix, disjoint staging, live-zero/NIL distinction, exact byte preservation, declared new flags, no disk recovery/name relocation. |
| HL-E6 stale lifetime | Application records/cell band retired; system band preserved; uint32 saturation and raw-number reuse caveat. |
| HL-E7 guest resets | Guest InitApplZone/SetApplBase/Launch/Chain unsupported; no safe-frame shortcut. |
| HL-E8 limit | Explicit host S/H_min/reference and checked bounded limit formula; ordinary manager checks/stack bounds unchanged; no historical numeric default or SetApplLimit binding. |
| HL-E9 flags/segments | Distinct states retained as future requirements; no purge/flag/segment operation added. Scrap's new flags are an original separate choice. |
| HL-E10 interrupts | Explicit known/unknown interrupt refusal before effects. |
| HL-E11 separation | No active execution; separately owned stop records/stacks/tokens; no whole-memory snapshot. |
| HL-E12 retained system references | Closed-world typed inventory; v1 refuses unknown registrations, guest patches and required system-storage scrubs; later closure/callback contracts deferred. |
| HL-E13 discarded bytes | No payload clearing promise; only application cells and exact planned destinations overwritten. |
| HL-E14 Finder information | Actual Finder/AppParmHandle/A5 population deferred; unsupported existing registrations refuse. A future admission must define stale-handle/+16 CLEAR/REBIND spans first. |
| HL-E15 Chain | Unsupported; future CODE-only loader, no native selection. |
| HL-E16 reporting | Phase/subphase, actual commit/epoch/writes/scrap/globals, unavailable tags and original diagnostic causes; no invented system error. |
| RA-E5 | Typed lifetime/actions/dependencies, staging and continuous coherence; no blind global-zeroing or universal stale detector. |
| PG-O5 | Unsupported fields remain unbound; before any later guest instruction, exposed views need continuous typed validity. No guessed ROMBase/version/Restart/MemError. |

All HL-Q1..20, LQ-1..15, AL-2/4/5 and RA-Q/ES-G/PG-Q plus earlier questions
remain historically open. Engineering decisions do not settle them. HT-A..F and
RA-S1..22 remain unexecuted. No new primary-source spot checks are claimed.

## Usage and verification classification

CLI-reported successful review totals:11502 input,8704 cached input,19131 output
(15469 reasoning included),39337 total tokens; costUSD estimate0.04832828.
The prior broad assessment reported292725 total and0.11941548 estimatedUSD.
Narrowing to these sections reduced observed total tokens by about86.6%, while
the different output/reasoning mix reduced the reported cost estimate by about
59.5%. This is two different tasks, not a controlled quality/billing benchmark.
No quota, invoice, cross-provider cost equality or subscription charge is inferred.
Retain Codex for decisions/integration and bounded Grok review for independent
matrices; do not repeatedly load both complete project histories.

Validation is document/source identity, link/registry guards and preservation
review only. No runtime tests, fixtures, compiler audit, CI implementation matrix,
VM/emulator/reference run or budget measurement belongs to this contract slice.
The separately owner-requested screenshots used the previously admitted f29113b
boot images and remain private; they do not execute or qualify SPEC-0030.

Next is a separately scoped original hosted implementation of this contract,
starting with inactive admission, plan/state/epoch ownership and scrap staging,
then commit/teardown and meaningful fault/refusal tests. Loader, callbacks,
historical probes and native applications do not follow automatically. Do not
advance existing verified build/edition or full128K claims on this document.

## Claude qualification follow-up - 2026-10-14

SRC-0149/0150 / [orchestration protocol](agent-orchestration.md) records one
Read/two model turns. Adopt D1 as overview clarification: the prior phase section
already required teardown preservation, so reject an assertion of wholly absent
scrap behavior. Adopt D2's explicit state/plan-lifetime transitions in revision2,
including fresh-preparation after changed inputs and certificate revalidation.
The stack-limit observation is a consistent refusal, not a proved defect. T1..8
remain unexecuted input; T2's unchanged oracle is commit-entry state, and T5 must
name its fault/write boundary. No fixture/TEST/IMPL/code or historical closure.
All initial HL/RA/PG dispositions and scope gates remain. Raw report outside Git.
