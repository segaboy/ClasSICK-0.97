# Paired release primary,286 progress alongside

2026-10-23;owner SRC-0390,ADR-0078.

The main deliverable remains paired-native-preview-2:ROM0.4.0-alpha.1 and native
PC0.2.0-alpha.1 with a moving cursor and one draggable/content-click original
window. Native Mac/x64 parity and existing release gates remain required.
The286 is an active secondary engineering lane;it is not a third acceptance
target that must boot before the Mac/PC pair can ship.

For each bounded work batch,identify the release prerequisite and an independent
286 gap. Put the substantial primary deliverable on the release path;add a
second worker only for that concrete286 gap or another distinct risk. Keep286
work bounded,with an artifact or explicit blocker/next unblocked step. Do not
create busywork,requalify unchanged tools,repeat old tests for progress credit,
or send both lanes to every provider. Codex remains sole writer/integrator.
All-three native performance,shared semantic behavior and scoped architecture
specialization goals persist. Note C16 width/span/helper impacts of shared
interfaces without replacing the main C11 targets with a weakest-target build.

Current batch:
- Release:[SPEC-0066](../specifications/SPEC-0066-window-foreground-coordinator.md)
  foreground coordinator implemented and locally tested;actual CPU/SCC/VIA
  exclusion/capture binding,total memory/stack and paired target qualification remain.
- 286:[SPEC-0067](../specifications/SPEC-0067-c16-portability-policy.md) reviewed
  C16 language/checked-arithmetic/helper contract;8 planned cases unexecuted.
  Next:original unlinked assertion/alignment facade and surface width proof,
  then checked segmented guest views/map. No loader/firmware/target run here.

[Current evidence](window-foreground-evidence.md). No new trial,installation,
native application ABI,reference input,release or publication permission follows
from the two-lane priority. Ordinary code/compiler/CI/Git/HQ work stays scoped;
OneNote/wiki remain retired,no additional chats/subagents or automation.

Follow-up2026-10-23: isolated SPEC-0068 supervisor exclusion callbacks now
implemented under IMPL-0069, source-pinned Windows/68000 CI passed; no ISR/MMIO/boot binding.
SPEC-0067 gains IMPL-0070 C99 assertion/chunk/offset facade and a surface-width
proof:9 final WCC calls/6 helper-free objects/3 deliberate refusals, no link/run.
General C alignment and segmented views remain open. [Checkpoint](cpu-c16-evidence.md).
