# Hosted resource residency contract review - 2026-10-14

SRC-0155..0158 / SPEC-0031 / ADR-0035. Contract only, unimplemented.
Lead self-review; human provenance review remains pending. No TEST/IMPL/build,
fixture, executed resource case, registry binding, loader or boot change.

## Scope and inputs

The owner answered "Do it" to the proposed Resource Manager contract before
CODE loading. Standing SRC-0149 permits bounded Claude/Anthropic and Grok/xAI
review of relevant reviewed public contracts/original code. Other gates persist.
Baseline public e2b0679, HQ f1ce00c; both tracked-clean with24 untracked public
research originals. Private baseline identities, byte-identical copies, packets,
prompts, streams and reports: `C:/ClasSICK/resource-residency-contract-20261014-a`.
Research originals are not read, modified, staged or sent to workers in this slice.

Read the binding policy/contribution/provenance guidance, existing SPEC-0023/0024/
0030, their original interfaces, public resource/launch lead reviews and runtime
plan. Reuse the reviewed SRC-0106 format/attribute facts and SRC-0130 motivation.
No new primary-source, sample, executable, asset or external implementation input.
The only new algorithm/state policy is original engineering design. Raw Resource
Manager traps and their historical identity/error/calling conventions remain later
work; no January1984 availability is inferred from this host service.

## Lead choices

| Topic | Contract disposition |
| --- | --- |
| Boundary | One borrowed immutable source, explicit host operations, no search/disk/guest trap/SDK or execution. |
| Residency | At most16 application copies, always locked/nonpurgeable/nonresource, within the existing32-record manager capacity. File flags remain metadata except reference/system-heap refusals. |
| Leases |32 concurrent tickets; same-key cache hits share the handle but issue distinct saturating IDs. Copied tickets do not add leases. Release retains residency; eviction/close needs zero leases. |
| Epoch/source lifetime | Bind the manager/lifecycle once; each OPEN supplies fresh source bytes/length and captures the current epoch. Close releases the borrow only on successful return; faults retain it. |
| Refusal versus effects | Validate before allocation. Ordinary memFullErr preserves guest state; successful allocation can compact other application objects. Unexpected faults retain actual allocations/copies/publications/disposals; no rollback. |
| Diagnostics | Native publication and guest effects are distinct. Actually allocated/published handles/tickets retain validity tags. Sixteen-slot disposal/retirement masks prevent stale fields from being described as resident. |
| Lifecycle | Authoritative FUTURE/REFUSE acknowledgment precedes OPEN ownership. Fresh inventory snapshots stay conservative through OPEN/BUSY/FAULTED. Only successful close allows closed reference-free native preservation. |
| Execution | Inactive ordinary context only. Data residency does not authorize code execution, active-root LoadSeg, callback return paths or guest handle mutation. Those need a later loader/ownership contract. |
| Accounting | Source buffer, loaded physical extents, cache/tickets and existing manager/lifecycle/session storage all count. New ABI/storage/frame sizes are unmeasured; no final128K/ROM claim. |

Implementation is not authorized by this contract slice. The next proposed scope
is an original hosted residency implementation and meaningful conformance tests,
before CODE parsing/loading and dynamic continuation admission.

## Claude review and dispositions

Claude Code2.1.287, requested effort medium, runtime claude-opus-5-5, completes
three model turns and exactly two Read calls. Successful result, no permission
denial or extra tool/file/command/web/MCP/write/subagent action. It reads only the
frozen new contract and lead boundary notes. Hashes are checked by the lead,
not by the worker. Four proposed acceptance cases remain unexecuted.

| Finding | Lead disposition |
| --- | --- |
| C1: DESCRIBE/RELEASE state missing in table | Adopt as clarification. General FAULTED rules already forbid continuation, but both rows now explicitly require OPEN; all former spans/tickets become quarantined diagnostics on fault. |
| C2: reopen could reuse a freed source | Adopt with changes. Binding was manager/map/lifecycle, not a permanent source, but OPEN's arguments were implicit. Name fresh source/length on each OPEN; rejected construction publishes no borrow/generation and leaves CLOSED. |
| C3: classification on reopen could remain NATIVE | Adopt with changes. Omission of a live owner was already a caller violation; make authoritative-registration acknowledgment, promotion-before-open, downgrade-after-close and fresh-snapshot ordering explicit. No new global interception exists. Preserved controls are immutable during lifecycle preparation, not forever. |
| C4: disposal before cache retirement leaves stale fields | Adopt with changes. Add sealed-slot disposed/retired masks and immediate ordered bookkeeping. Stale fields with a disposed bit are tombstones. Recovery disposal remains forbidden, even if an old field or mask exists. |

The suggested cases cover allocation/movement followed by fault, unchanged OOM,
ticket copy/replay/foreign/reuse and close fault/reopen. They are planning input,
not delivered fixtures or executed TEST cases. Fault injection must name exact
semantic units and report actually completed native versus guest effects.

## Grok review and dispositions

Grok Build1.0.52, requested reasoning effort low, runtime grok-4.7-build,
completes two turns and exactly two read_file calls. Final end_turn, no other
file/tool/command/write/web/MCP/subagent action. Twelve requirement rows and
three claimed scope leaks are review input, not historical evidence.

| Review item | Lead disposition |
| --- | --- |
| Open failure/diagnostic/capacity names | Adopt. SOURCE leaves CLOSED and publishes no borrow/generation. Distinct cache/ticket/manager/profile/counter causes and safe unresolved-reference fields are explicit. |
| Other attribute bits not repeated | No defect. Only0x40/reference affect admission; all others are raw metadata. No unstated flag execution semantics are required. |
| Positive profile maximum exceeds actual guest capacity | No defect. Profile maximum is an upper admission limit, not an allocation proof. Existing checked map/manager bounds and physical rounding determine actual capacity. |
| Mid-close per-slot state | Adopt with changes together with C4; masks qualify sealed cache slots, not guessed raw-handle ownership. |
| G1: external source norms/system-reference label | Adopt wording refinement; reject an inference that omitted reviewed records are unavailable or new implementation authority. Source records are motivation/provenance, requirements are explicit. Replace the imprecise label with the existing FUTURE/system-scrub boundaries; no bit7 resolver is supplied. |
| G2: empty logical span versus invalid zero window | Adopt as clarification. Return EMPTY/NULL/length0 without a zero-length guest window, while separately validating the live two-byte physical reservation. |
| G3: drop application-only compaction restriction | Reject. Packet synopsis omitted the selected-zone qualifier. Independently check own manager.c: allocation chooses zn from TheZone/SYS and compact filters blocks by that zone. Application admission with no SYS flag cannot compact system objects. Keep actual movement reporting and application-only scope. |

Lead follow-up additionally specifies operation-dependent failure precedence,
native publication versus guest effects, counter checks only when issuing IDs,
cache-hit capacity distinctions and unexpected native manager failure even before
allocation. Saved fault reports do not issue new data views. No worker re-review
of these final edits or independent provenance certification is claimed.

## Identity and usage

Both frozen packets contain SPEC-0031 SHA256
`635c467c90c04c6969a84dfa8392c20df200e77d4bc3afa57f498e626e049477`
and boundary notes SHA256
`69f3a312b360325c0c0ce1c6e1a47b07f049bcb4bf32108b38b2e85fe93542f4`.
The final lead-amended contract differs from that reviewed packet; raw copies
remain unchanged privately. Reports are identified in SRC-0156/0157.

Claude reports25373 total tokens (10915 output including8385 thinking), list-price
estimate0.2965314USD. Grok reports26492 total (9516 output including7983 reasoning),
estimate0.02717008USD. These are client estimates, not invoices, additional
subscription charges, quota availability or a controlled cross-provider benchmark.
Reports1044/989 words exceed the requested800/850 targets; turn/tool boundaries
are verified, strict word-limit enforcement is not claimed. No endless retries.

## Verification and remaining gates

This slice checks document/provenance identities, links, registry constraints,
staged content and preservation. Runtime/compiler/CI execution and budget claims
remain those of the prior heap checkpoint, compiled89e3bbb/public e2b0679/HQ f1ce00c.
No runtime test or Resource Manager case runs, no new build is allocated, and no
verified/edition/native-image pointer advances from this design. All24 originals
and private copies remain unchanged/untracked; code, interfaces, boot inputs,
assets, prior evidence/build/edition records stay intact. Git wiki queue only.

RF-1..9, HL-Q1..20, AL/LQ/RA/ES/PG and seven-draft MQ/WQ/SQ/OD questions retain
their historical status. No guest Resource Manager/LoadSeg ABI, CODE execution,
system resources/search, purge/writeback, MFS, callback closure, native app,
VM/emulator/reference trial, UART body work or full128K/physical/parity/edition/
B2 criterion6/human-review closure follows. Normal owner continuation precedes
the proposed implementation, with standing scoped reviews available.
