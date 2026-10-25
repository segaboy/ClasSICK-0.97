# Original first-input checkpoint diagnostic

Windows11,baseline83f7b6f. Codex sole writer/integrator/verifier. One bounded
original-public task;one frozen read,no other reads,commands,writes,web,MCP,
delegation or tests. No manuals,HQ,untracked originals,Apple/MS implementation,
emulator source or examples. Return usable artifacts only,not executed tests.

Deliver platform/mac128k/input-probe.h,tests/mac-input-probe.c,and exact unique
old/new replacements for platform/mac128k/window-entry.c. JSON
{files:[{path,content}],replacements:[{path,old,new}],handoff:{result,
tests_executed:[],blockers:[],decisions_needed:[]}}. Total proposed C<=130 lines.
No builder/CI/spec edits or stable IDs. No vacuous assertions,placeholder
expectations or guessed fixture rules;report a missing fact as a blocker.

Purpose:freeze original diagnostics before later GUI actions can contaminate
the post-input record. This is a new opt-in observation profile,not a motion
fix or qualification. It changes no acceptance policy or normal image.

Header:include stdint. CLASSICK_MAC_FIRST_INPUT_PROBE defaults0;reject values
outside0/1. Define CS_MAC_FIRST_INPUT_LIMIT as5000u. A pure static inline
int cs_mac_first_input_ready(uint32_t services,uint32_t losses,
uint32_t deliveries) returns true iff losses!=0 OR deliveries!=0 OR
services>=CS_MAC_FIRST_INPUT_LIMIT. No globals,allocation,IO or side effects.
The helper exists for independent host checks even with the profile default0.

window-entry.c:include the new header. After existing IDLE_PROBE validation,
reject FIRST_INPUT_PROBE=1 without IDLE_PROBE=1 (the latter supplies the existing
checkpoint symbol/assembly). Codex's builder will set both for this new mode.
Inside observe(),after the existing complete trace copy while IPL7 is held,
use #if FIRST_INPUT_PROBE and call the existing noreturn
cs_mac_window_idle_checkpoint only when cs_mac_first_input_ready(
cs_mac_trace.services,cs_mac_trace.losses,cs_mac_trace.deliveries) is true.
Use #elif IDLE_PROBE for the unchanged old >=500 idle condition. This is
critical:the first-input profile must NOT accidentally halt at500 services.
Retain the existing !dead gate and no restoration before diagnostic spin.

Default paths,assembly,trace128B layout,MMIO,owner calls and timing policy stay
unchanged. No new BSS. The shared checkpoint label does not imply semantic
shutdown. Threshold5000 is a service-count fallback,not elapsed-time units.
Once frozen,the record can establish first delivered sample/loss/idle only
with a guarded own PC/IPL7;host receipt alone never proves guest acceptance.

Tests:standalone original C test over literal boundary cases0,4999,5000,5001,
UINT32_MAX,first loss,first delivery,saturated counters and combinations.
Concise failure output/nonzero exit. No target run;Codex handles native twins,
normal/probe byte identities,mode refusals and later authorized trials.
