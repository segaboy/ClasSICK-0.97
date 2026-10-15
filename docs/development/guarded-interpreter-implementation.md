# Guarded interpreter X-1 implementation - 2026-10-15

SPEC-0034 / ADR-0038 / IMPL-0038 / TEST-0042 / SRC-0183..0186.
[Executed evidence](guarded-interpreter-evidence.md). The original full-bridge
contract remains unimplemented: this record implements its X-1 dependency only.

`core/cpu/m68k-guard.h` exposes `cs_m68k_bus_step_guarded`. Existing bus/options/
bounded entry points and structures retain their APIs; result6 POLICY is appended.
The actual existing decoder and private transaction are shared, not predicted by
another decoder. No host allocation, process mutable global, ISA extension,
loaded-image EXEC override, session driver or native guest-selected callback.

The native guard is mandatory, immutable, pure, non-reentrant and side-effect-free.
Its explicit context extent covers all direct policy data. Event and frame
pointers are borrowed only during the synchronous call. This is a trusted native
interface, not a sandbox for hostile callbacks or discovery of hidden pointers.
Any indirectly referenced controls and complete policy roles need consumer-side
admission; X-2..X-5 will supply the session/image boundary. No guard defaults to
allow-all, but a native caller can explicitly supply a permissive guard.

Before touching even the report, the API checks representable/aligned known
native objects, context pointer/size agreement, map validity and pairwise controls/
output/context/backing aliases. Distinct backing regions may not physically
overlap, even read-only aliases. All protected controls are conservatively disjoint.
Native storage must still be live, truthfully sized and synchronized; no pointer
probe can establish those properties. Invalid envelopes return ARGUMENT and keep
report/frame/RAM/gateway bytes. This path deliberately gives malformed maps the
same no-output ARGUMENT refusal. A safe envelope then permits ordinary diagnostics
on CPU/address/policy refusal. Policy denial yields POLICY with the attempted
span, access and actual decoded word; memory status need not indicate a fault.

The callback receives whole actual spans before opcode, extension, explicit or
implicit read/write staging and taken-target probes. Access names are distinct:
a target word probe is TARGET, not a second OPCODE event; RTS stack data is READ.
The first opcode event has word0; the INSTRUCTION event has the fetched word and
precedes class dispatch. Every A-line is then hard-refused before either legacy
native gateway or dispatch hook, irrespective of policy approval. Unknown/F-line/
TRAP/exception opcodes retain unsupported OPCODE without service execution.
The guarded API has no options/dispatch hook and uses strict24; map permissions,
whole-one-window extents and word/long alignment remain mandatory. No fetch from
non-EXEC owned CODE is admitted by this primitive.

MOVE/MOVEA, register increments, flags, branch/control targets and pending stores
use the same staged frame/store as existing execution. Taken target probes happen
before BSR/JSR stack staging and final commit. Initial and proposed A7 are even
and inside the required stack limits. Initial/proposed SR masks require S1/T0/I7;
CCR bits remain available. Register-only MOVEA A7 cannot escape. COMMIT sees both
the original and complete proposed CPU frame after fixed checks, with proposed
PC/zero bytes/zero access, before the first RAM or caller-frame write. A denial
withholds every effect of this instruction; effects of earlier successful steps
remain. There is no post-write rollback or native service in this guarded path.

The caller supplies actual range/class/state rules; no segment/world ownership,
sealed inventory, lease, return-token exemption, budget, session entry or retirement
is inferred. Existing unguarded entry points remain intentionally usable by their
prior consumers and cannot be substituted for the future guarded bridge.

## Worker and lead review

Claude authored the primary290-line original test artifact from public API/own
fixture ranges; Codex independently reviewed expected encodings/failure effects,
integrated it and added pure before/proposed-state checks and all4096 A-lines.
Grok reviewed only the commit/access/service boundary and found no demonstrated
bypass, with explicit hidden-pointer/native-hook and dependency coverage limits.
Its phrase SR==2700 is corrected to masked S/T/I equality, with CCR tests passing.
No extra review call was needed. Neither worker ran tests or wrote the repository.
Actual read ranges/input/output identities and usage are in the evidence JSON.
Two CLI calls are not a scope-normalized efficiency benchmark or quota guarantee.

Next is X-2: immutable policy binding through session initialization, entry/token
push, RTS validation and stop. It must retain all primitive checks, remove
unguarded/trusted-executor substitution, and define precise return-token authority
before any controlled image is admitted. X-3 lease/inventory, X-4 owned fetch and
X-5 handback remain separate; full application compatibility and both ROM/System1
boot directions remain untested. No new trial or next implementation is assumed.
