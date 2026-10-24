# First native PC motion packet diagnostic

Windows11, paired-native-preview-2, baseline88cbdee767b7dd19b8ad342a1067ec18e8397bdc. Codex is sole repository writer/integrator. One qualified Copilot Sonnet5.5 low primary, no fan-out/requalification. Use only the frozen original public inputs below. No other paths, commands, edits, web, manuals, research originals, HQ, credentials, UART bodies, Apple/Microsoft/emulator implementation, delegation or execution. Tests are proposed, tests_executed=[] required.

Actual result: both source CI jobs atb3cb3fe passed. Native PC normal GUI clicks previously incremented0002, but captured Windows dragging clamps the guest pointer at511,341. Existing first-LOST probe never reaches its spin. Device-API dragging passes separately (E2), not Windows-GUI E3. We need the first actual decoded movement packet, not a speculative decoder/policy fix. No current runtime policy change is authorized by this assignment.

Deliver an opt-in compile-time motion mode for the existing SPEC-0073 original probe. `CLASSICK_PC_INPUT_PROBE_MOTION` defaults0 and rejects values other than0/1. Default0 retains EXACT existing first-LOST behavior and mailbox. Mode1 returns0 without effects for NULL, NONE, LOST, or PACKET with both dx/dy zero. It stops only on PACKET with a nonzero dx or dy, AFTER the unchanged session poll/dispatch through the existing const observer. No extra IO, graph/trace/pixel mutations, allocation, state/callback or input-policy change. Existing backend/observer ABI stays. This is a nonreturning measurement, not semantic STOPPED or release code.

Motion-mode words loaded into r8..r15 by the unchanged backend:
0: 0x4353504D4F543031 (original CSPMOT01 magic)
1: high32 bus.packets, low32 bus.errors
2: high32 decoder.header; low32 ((decoder.x &255)<<8)|(bus.last_byte &255), raw X/Y bytes
3: high32 (uint32_t)delivery.packet.dx, low32 (uint32_t)delivery.packet.dy (two's-complement encoding)
4: high32 delivery.packet.buttons, low32 owner.pointer.down | (owner.pointer.suppress<<1)
5: high32 (uint32_t)owner.pointer.x, low32 (uint32_t)owner.pointer.y, POST dispatch
6: high32 (uint32_t)owner.window.x, low32 (uint32_t)owner.window.y
7: high32 owner.product.inputs, low32 owner.window.dragging

Raw bytes are the actual parser fields retained from the completed packet, not reconstructed from decoded deltas. No history/host-cause claim from one packet. Before native register interpretation Codex verifies exact own spin RIP AND selected-mode magic. Normal previews exclude probe modules entirely; normal image byte equality must be checked by Codex. The build flag/wiring/package metadata and native verification are Codex's small integration work, not your deliverable.

Return original exact unique replacements for platform/pc/input-probe.h/.c and full new tests/pc-input-motion-probe.c. Tests should intercept the noreturn backend via setjmp/longjmp, never execute native loop, check no effects over the graph and sentinel trace/context on ignored paths, exact independently literal-encoded words for signed mixed-axis reports/negative positions/counter saturation/raw-byte masking/non-left buttons, X-only and Y-only triggers, and unchanged graph/context/trace on capture. Keep existing tests/pc-input-probe.c unchanged. No tests that only compare implementation-generated expectations. Keep <=240 changed code lines. JSON only (no fences): {replacements:[{path,old,new}],files:[{path,content}],handoff:{result,actionable_findings:[],tests_executed:[],blockers:[],decisions_needed:[]}}.
