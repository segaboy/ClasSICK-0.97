# Owned CODE image contract review - 2026-10-15

SRC-0171..0174 / SPEC-0033 / ADR-0037. Contract only, unimplemented.
Lead self-review; human provenance review pending. No new TEST/IMPL/build,
fixture, code, application execution, boot change or VM/emulator/reference trial.

The prior preflight checkpoint is complete: compiled724a72d, public5486c1d and
HQ3dcc4bd, with1911 local checks and2361 publication CI checks. Those are prior
checkpoint results, not execution of this contract. Source/evidence/build/verified
pointers remain exact. Both mandatory System1/ROM boot directions remain untested.

## Inputs and decisions

Only reviewed own contracts, SRC-0130's checked LK dispositions, original
manager/session/CPU/guest-patch interfaces/code and a lead API synopsis are used.
No new manual/source/sample, raw original, protected executable/resource/asset,
HQ worker input or emulator implementation. Existing JMP absolute-long encoding
is generated as owned loaded-table data; nothing is run. AL-2/LQ-1..15/R-001 and
earlier historical questions remain open.

| Choice | Original engineering boundary |
| --- | --- |
| OI-1 | Exclusive eager image handles; do not promote resource-cache tickets or duplicate cache copies; associated caches must be CLOSED on the same manager/controller/map |
| OI-2 | Stage CODE0 header/table plus all selected logical segments;64 entries/16 segments, at most524816 staged bytes, checked explicit actual capacities |
| OI-3 | Successful PREPARE ends original fork borrow; native staging remains until successful LOAD/CANCEL or PARTIAL RELEASE, and survives FAULT |
| OI-4 | Current ApplLimit equals fresh layout's candidate, full-zone stack restriction preserved; capacity accounts only for physical extents below that limit, with no placement reservation |
| OI-5 | First allocator success commits effects; first ordinary OOM stays PREPARED, later ordinary OOM becomes PARTIAL for explicit RELEASE, not automatic rollback |
| OI-6 | No EXEC on application/world/stack destinations, nor physical aliases to any executable region; existing bytes/map flags cannot silently grant entry |
| OI-7 | Each segment locks via actual CS_MM_HLOCK; populate zeroed A5 data and eight-byte loaded JMP rows only after all segments copy successfully |
| OI-8 | Every segment, including CODE1, stays pinned for entire image lifetime; no individual unload/purge or current execution admission |
| OI-9 | RELEASE checks all actual owned objects; LOADED clears world then disposes reverse order, PARTIAL preserves unwritten world and cleans acquired handles only |
| OI-10 | Unexpected invariant faults retain immutable first diagnostics and actual masks/tombstones; no generic FAULT recovery/reset bypass in v1 |
| OI-11 | CANCEL remains ordinary/inactive but skips manager/resource integrity calls when retiring a genuine pre-allocation preparation; no stale-handle lookup |
| OI-12 | No launch globals, Finder object, stack token or CPU entry construction; full execution bridge/continuous validity/full128K/physical/boot qualification stay separate |

## Bounded worker dispositions

Both complete prompts were shown before launch under standing SRC-0149 transfers.
Each worker reads only its frozen reviewed-public proposed contract/API synopsis,
one Read in two model turns, no extra files/commands/edits/web/MCP/subagents/tests.
The packet pins initial SPEC bytes19434 and SHA256
91da2b781e1053eece589c6954138f424e920668c1c5c63fcc13f26a41742e67.
The final amended contract is a lead revision, not an externally re-reviewed one.
Both reports exceed their requested450-word bound; call/input boundaries are
verified independently. Their observations are not authoritative or certification.

| Finding | Lead disposition |
| --- | --- |
| Claude C1 / Grok G1, ordinary later OOM permanently blocked | Adopt with changes: explicit PARTIAL and validated inactive RELEASE for ordinary allocation capacity only. Keep unexpected native/invariant faults terminal for this coordinator, with no guessed recovery |
| Claude C2, residual code may be executable | Adopt with changes: refuse destination EXEC and every physical executable alias at PREPARE and retain binding. No unconditional payload scrub is promised; allocator disposal does not erase bytes and scalar addresses are not capabilities |
| Claude C3, CANCEL context exemption ambiguous | Adopt: all operations require ordinary inactive context; CANCEL only skips applicable manager/resource integrity calls |
| Claude C4, memFullErr might write guest MemError | Reject as a defect: own manager returns -108 in native cs_mm_reply.code with no guest/global write on OOM; the supplied synopsis already stated no RAM/record effects. Add exact native-result wording |
| Claude C5, lock API not named in synopsis | Adopt precision, not an absent prerequisite: existing CS_MM_HLOCK goes through cs_life_manager_call; unexpected failure after allocation retains actual committed allocation |
| Claude C6, disposed borrow could identify reused handle | Adopt clarification: tombstones are scalar-only diagnostics, never dereferenceable leases; retained staging/control/world storage is a different lifetime |
| Claude uncertain reader/CLOSED or failed generation | Clarify: direct cs_rf_open/find via SPEC0032 is independent of CLOSED cache; failed PREPARE consumes no generation |
| Grok G2, source-free LOAD needs source for layout | Reject: SPEC0032 layout already consumes copied manifest, not source. Explicitly name cs_code_check_layout and revalidate staged headers/scalars; do not retain a fork unnecessarily |
| Grok G3, limit accounting and stack below configured end | Adopt explicit ApplLimit-limited extent formula. Reject its claimed reachable example: stack inside full app zone is UNSUPPORTED; stack entirely below app fails the cell-band/candidate-limit inequality. Current admitted layout has candidate=configured end |
| Grok G4, B too small for table and row masks missing | Reject reachable-counterexample claim: B>=above>=table_offset+8*entries>=32+8*entries is already implied by actual preflight. Separate table-byte progress and segment masks already exist; CODE0 allocates no17th handle |
| Grok G5, lock/copy failure could claim no guest effect | Adopt explicit persistent guest_committed tag after allocation; not a contradiction in the original first-allocation boundary. No allocated object can return to CANCEL |
| Grok G6, omitted inventory/EXEC could bypass ownership | Preserve compulsory coordinator gate and adopt non-EXEC/physical-alias restriction. A caller bypassing independent low-level APIs violates the protocol; this contract is not a sandbox or automatic interceptor |

The next original implementation may exercise the explicit boundary/capacity/
source-lifetime/PARTIAL/alias/fault/mask obligations in SPEC0033. No case is created
or executed here. Future execution must separately establish all fetch bytes,
guest/service/manager writes and transitive return quiescence, then explicitly
extend this profile's non-EXEC restriction. Existing sessions/Toolbox/heap/cache
gates are unchanged; no dynamic LoadSeg or guest launch binding follows.

## Identity, usage and preservation

[Review manifest](code-image-contract-review.json) pins packet/report/final-contract
identities and actual reported usage. Claude2.1.287 reports claude-opus-5-5, low
effort,19137 tokens total including3472 output/1909 thinking, private report4008
bytes SHA256b26d64d22d3e59b319c95d558bd64d57ae371a7cc36ffd4f1fb2d6868d675648.
Grok1.0.52 reports grok-4.6-build, low effort,20767 tokens total including5403
output/4318 reasoning, private report4431 bytes SHA256
d6c09304480a9c3ae855b567a77228c229869b80326f6edeec0cd51eb07a088e.
CLI list estimates0.1574444USD and0.0183362USD are not billing/quota or a comparative
benchmark. Prompts/raw streams and byte-identical24 original copies remain outside
Git; originals stay unchanged/untracked. No extra agent/chat/automation/OneNote.
Every old compiled/interface/boot/assets/UART/build/edition/project pointer and
human/source/native/physical/full128K/parity/B2 gate remains exact.
