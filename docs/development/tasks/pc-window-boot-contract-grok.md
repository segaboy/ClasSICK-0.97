# Grok PC window boot-binding contract prompt

2026-10-22;SRC-0347;one frozen public-only primary. Revision `3cf2b723c9388890a7cc2808f948abb518b0935e`;45866 bytes/652 lines;SHA-256 `b82cbc0b6b5926afe3c330ab4932951a69fab7e36c5648f6b2e4f4214d47f32a`. Exact selected paths/ranges/hashes in the [receipt](../pc-window-boot-contract-review.json);raw packet/transcript outside Git.

```text
You are a supervised read-only worker on ClasSICK 0.97, Windows 11.
Deliver ONE useful original engineering contract draft for the next PC native
draggable-window boot binding in paired-native-preview-2. Codex is sole repository
writer/integrator/verifier. This assignment is contract/case design only: no C,
headers,fixtures,implementation,builds,tests or target run. No historical claims.
Use only the frozen reviewed-public original inputs below through EOF. No tools,
file discovery,commands,edits,web,MCP,subagents or other files. No HQ,manuals,
untracked research originals,UART bodies or third-party implementation supplied.
Existing specifications/helpers stay unchanged. No new hardware recipes.

Owner accepts this contract slice with "Do it." Mac/PC native feature parity,
external native-app gate,Mac M1-M7 source blockers,PC GUI/human capture gaps,
128K/B2/source/role/UART gates and exact-image trial/release permissions persist.
Current releases stay immutable. BIOS/shared-payload goal is future;do not design
a BIOS loader or common binary handoff ABI here. Neutral shared scene/input code
must not acquire UEFI headers. Current optional UEFI profiles keep behavior.

Required design decisions and useful boundaries:
- Separate a firmware-neutral PC RAM/presentation/session boundary from a thin
  current-UEFI gate/loop/profile. Preserve real exited/status0,owned-memory/mapping,
  native descriptor readiness,framebuffer/map/RSDP/PMtimer/ACPI8042 checks. Not every
  64-bit PC is thereby supported. No firmware services after exit. Existing UART
  bodies and their observer ABI cannot be changed or copied;new explicit path may
  bypass them,with that diagnostic limitation disclosed.
- Existing compositor/cursor need ordinary-RAM surface;never bind their storage
  directly to volatile framebuffer. Existing cs_fb_present accepts RGBA8 ONLY.
  Use a fixed logical512x342 Mono1 canvas (64-byte stride,21888 bytes),48-byte
  cursor payload and a bounded one-row RGBA conversion workspace of4*physical
  width <=32768 bytes,not a full physical-frame RAM image. Propose a separate PC
  presentation bridge that uses existing cs_fb_init/cs_fb_present unchanged.
  Target at least512x342,center unscaled at floor((W-512)/2),(H-342)/2. First
  successful publication initializes all visible margins white,preserves pitch
  padding/tail,and later full logical-canvas publications leave margins white.
  Color black/white only,device reserved byte0. No device framebuffer reads.
- Use the hosted recipe:window(80,70,224,144,title18),original7x7 plus/hotspot3,3,
  product(value97,inputs0,progress16,phases6,success1);shared owner centered256,171.
  Allocate workspace once from exclusive owned storage within existing131072-byte
  arena;no reset/reuse while borrowed,no per-report allocation. Compute fixed
  buffer costs vs all state/alignment/stack/linked-image costs still to measure.
- Exactly one controller poller calls cs_ps2_bus_poll_delivery once per turn;
  drain keyboard queue boundedly (existing capacity8). PACKET goes once through
  cs_pc_window_packet;LOST goes once to owner.cancel;NONE has no input-owner call.
  Shared owner press count alone feeds UI. Keyboard Space/Escape must not add
  synthetic mouse clicks or terminate the continuous new preview;old modes keep
  their behavior. No freshness from packet/error/count totals.
- Define unknown initial button state conservatively. Need visible initial scene
  without fabricating an observed button level or counting a held-at-start press.
  Candidate:declare neutral initial owner,then explicit initial cancel before
  enabling reports,so LEFT remains suppressed until observed release. Resolve and
  explain this engineering policy;do not delay all display until first mouse packet.
- All accepted owner calls may change cursor/bookkeeping even when returning OK0;
  a successful accepted operation must publish coherent latest RAM state. No idle
  repoll/replay/double count. Distinguish owner fault with retained semantic/render
  prefix from presentation fault after a complete RAM frame. No automatic repair,
  re-begin,rollback,count decrement or stale restore. Define known loss,cancel,
  terminal controller/clock/queue failures,partial MMIO publication,stop/retirement
  and truthful trace progress. Arbitrary hardware exceptions are not recoverable
  just because E1 helpers can return errors. No new trace binary layout is needed.
- Future profile is explicit opt-in,mutually exclusive selected/outline/window;
  continuous until explicit external stop/fault,not hidden60s. Existing tests/CI
  cannot qualify changed image;later native compilation/budgets and fresh exact
  image permission are needed. Same normalized state/pixels must match Mac native
  path when its adapter is admitted. E1 synthetic,E2 device,E3 GUI,E4 human separate.

Return JSON keys spec_md,decisions_md,cases_md,handoff. Total <=220 lines/2400 words.
spec_md:ready-to-adopt concise behavioral contract,including module boundaries,
preflight/effect ordering,startup/poll/publish/terminal lifecycle and budgets.
decisions_md:8-12 numbered B- proposals with rationale/open decisions and a small
future implementation sequence;lead assigns IDs. Freeze behavior,not invented
header layouts. cases_md:24-32 uniquely labeled WB case rows with trigger and
observable expected result;all explicitly PLANNED/UNEXECUTED,no fixtures/code.
handoff:result,actionable_findings,tests_executed=[],blockers,decisions_needed
(under180 words). Scope all contracts/cases honestly;worker suggestions require
lead review. EOF after supplied exact source excerpts.

```

Completed118.48s,73 lines/1659 words;zero observed tools/writes/tests.
[Lead dispositions](../pc-window-boot-contract-review.md);no retry/second provider.
