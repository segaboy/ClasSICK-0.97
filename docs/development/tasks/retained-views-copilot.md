# First retained guest-view inventory for the 286 lane

Windows11,public baseline3e35bbd. Codex sole writer/integrator/verifier.
One Copilot Sonnet5.5 low primary,one frozen packet only. No commands,other
reads,writes,web,MCP,delegation or tests. No manuals,HQ,untracked research,
Apple/MS code/media,emulator source or sample code. Reuse qualifications.

The paired native Mac/x64 release remains primary;desktop-dependent input
trials are deferred while the owner uses the computer. SPEC-0069 section6
names retained-view/callout inventory as the next bounded286 prerequisite.
Do this explicit first slice: guest.h,m68k-call.h/.c,toolbox.h/.c only. Other
modules are UNREVIEWED,not absent. No runtime/API/ABI/backend implementation,
new benchmark,compiler/emulator/reference/hardware run or release permission.

Deliver docs/development/retained-guest-views.md plus .json as review proposals.
Identify every native guest view/raw backing pointer in these scoped headers
and functions: acquisition,where actually dereferenced,any external call
between acquire/use,retained controls,who owns the borrow and why. Separate
local validation-only/consumed-before-call spans from spans read by a handler
or written after it. Merely being in scope is not a post-call dereference.
Distinguish trusted native handler invocation from call-into-guest execution.
The Toolbox handler contract forbids mutation/reentry/retention;do not invent
an existing flat-code bug or guest-callback path there. kept metadata is
separate from these temporary native borrows. Map immutability does not itself
define a future segmented backend's chunk-pointer lifetime.

C16 facts already qualified/reviewed: size_t/ptrdiff are16bits,the native chunk
limit32767;logical guest addresses/lengths stay32bits with24bit guest address
limits. SPEC-0069 models bank descriptors/capabilities,not actual far pointers.
No arbitrary far addition,physical alias proof,general C alignment or native
commit guarantee is qualified. Adjacent logical regions do not merge.

For each row propose C16 treatment (per-operation logical access,explicit
capability,owned bounded copy or deferred choice) and preserve native68000/x64
fast paths. Do not prescribe per-byte callbacks or wholesale staging for all
targets. Derive any byte bounds from supplied constants/width() only and label
them arithmetic bounds,not measured stack/RAM budgets. Keep preflight/effects,
staged writes,immutable active map and current error/fault semantics.

Recommend the first small contract/API prerequisite and list blockers before
qualified native bank access. Retained resource/source/cache views,lifecycle,
guest patch routes,selected-drawing state,full instruction bus and physical
backend are outside this slice. No claim of complete production coverage.
Do not assign stable IDs or present proposals as adopted contracts.

Return JSON ONLY {files:[{path,content}],handoff:{result,tests_executed:[],
blockers:[],decisions_needed:[]}}. Markdown <=1000 words;JSON <=14 concise rows
with exact path/line/function evidence,retention boundary,classification,
proposed_C16_action,confidence. JSON may include limits/next_contract/open_gaps.
The two artifacts must agree;no verbose duplicate transcript or scope expansion.
