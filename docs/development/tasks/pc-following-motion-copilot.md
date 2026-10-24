# Bounded PC following-packet diagnostic

Windows11, reviewed public baseline a98788192b041c3186062d20afe93aabefb004b7.
One Copilot Sonnet5.5 low primary; Codex sole writer/integrator/verifier.
No commands, writes, other reads, web, delegation, MCP or tests. Frozen inputs
below are original reviewed public project artifacts, not manuals/research/HQ.
Return usable exact replacements as JSON; do not inspect the checkout.

Actual own native first-motion capture: packet1/errors0,dx255/dy-255/buttons2
for a one-pixel automated gesture. This may include initial capture/positioning.
It does not prove the next packet or general host cause. The supported GUI drag
interface has window-coordinate endpoints and no button or relative-delta
selector. No tool implementation or emulator source was inspected.

Deliver a diagnostic-only `motion-later` variant of SPEC-0073. Exact semantics:
- New CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET defaults numeric0; reject other
  than numeric0/1 (including empty/identifier). Value1 requires MOTION=1.
- In motion mode when SKIP=1, return0 unchanged if bus.packets<=1. Otherwise
  keep the existing nonzero PACKET trigger/mailbox exactly. This observes the
  first nonzero packet whose TOTAL decoded-packet counter>=2, not necessarily
  the second motion packet. No private counter/history/extra IO/mutation.
- Default loss and default motion semantics and word layout stay byte-for-byte
  equivalent; source field lifetime correction stands: header/X are already
  reset before PACKET; only last rawY retained. No rawheader/X reconstruction.
- Builder ValidateSet accepts motion-later, requires explicit probe like motion,
  emits MOTION=1 and SKIP_FIRST_PACKET=1 for motion-later only. Existing metadata
  string is sufficient, no new parameter/schema. Package accepts consistent
  motion-later metadata, preserves all pre-write rejection controls.
- CMake adds a third executable/CTest using the SAME motion test source with
  both macro flags1. Include it in strict compiler/sanitizer loops.
- Extend existing literal tests with exact count0/count1/count2/saturation
  boundary and nonzero X/Y, NONE/LOST/zero-motion below/above threshold; no
  graph/context/trace mutation. Existing35 checks retained in both builds.
  Default mode still captures at packet1 (and does not acquire a new count
  gate). Later mode ignores0/1 and captures at2. Include realistic reset parser
  fields and a literal eight-word count2 oracle; old synthetic field tests
  remain explicitly packing tests. Keep unexpected-backend armed guard.
- No changes to native backend, decoder, session, dispatcher, regular inputs,
  normal build or existing legacy test source. No runtime source other than
  probe.h/.c. No new API/trap/native-app/release/VM/hardware authority.

Return JSON ONLY: {replacements:[{path,old,new}],handoff:{result,
actionable_findings:[],tests_executed:[],blockers:[],decisions_needed:[]}}.
Every old string must occur exactly once in its provided original file.
Use <=260 changed code lines. No whole CMake or builder replacement. You do not
execute tests; Codex will compile, exercise negative controls, verify normal
binary equality and rebuild committed native source before any isolated trial.
