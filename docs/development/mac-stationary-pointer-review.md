# Stationary Mac pointer - lead source review

2026-10-26;SRC-0496. Reviewed code49b619c;SPEC-0075 and original timer-poll.c,
polled-phase.c,window-entry.c only. Copilot41.89s/30812B,one view,no execution,
commands,writes or proposed patch. This is an incomplete source review,not a
hardware audit or proof that the code is defect-free.

| Finding | Lead disposition |
|---|---|
| F1:service flow appears consistent | Retain within supplied subset. No target/root-cause proof. |
| F2:decoder only emits displacement on observed DCD changes | Retain code fact and missing-observation hypothesis. Do not infer live DCD,missing edges or wiring. Inversion changes direction;it does not itself turn accepted nonzero displacement into zero. |
| F3:trace lacks sufficient activity/delivery distinction | Adopt with changes. Samples includes button-only records;count admitted nonzero motion separately. No current trace was obtained. The physical timer candidate runs normally,not the500-service idle control. |
| F4:ACR0x40 | Keep existing exclusive ownership/contract. Reject unconditional harmlessness claim about unreviewed modes. |

Next experiment is an original,versioned diagnostic contract that distinguishes
raw movement-phase observations,stable-pair acceptance/uncertainty,nonzero motion
queued,and motion delivered. Include relevant previous/current triples,baseline,
loss/fault reason and pointer position. Observe PB4/PB5 and DCD changes separately
from button-only activity. No missed-edge count or causal hardware claim.

Require a distinct,strictly decoded format;preserve MWP1/MWR1 offsets and old
decoders rather than silently reusing rr3/snapshot slots. Fit/cost/native-image
proof and normal byte identity precede any trial. A trigger must avoid ending on
startup baseline or a button-only sample before motion. Choose an explicit
movement/loss trigger and bounded fallback with honest timing;do not assume a
clock frequency or repurpose the old500-service idle probe without review.

Existing host Snow readout is the target for this experiment. Direct mouse on
Windows is not physical Macintosh hardware. No new readout method,manual source
or hardware research is authorized by the worker's proposed approval question.
No new owner approval is inferred as necessary merely because a worker asks for
one. Current original-controller/diagnostic authorization and existing gates apply.

Before implementation,review exact timer/motion/inbox types,the original native
checkpoint veneer and builder restrictions. Tests should distinguish button-only
activity from movement and loss,prove old formats refuse the new one,check every
trigger/refusal and register packing,and retain native BSS/default-byte gates.
These are proposed tests,none executed here. No reload/sign/address/uncertainty
change or speculative controller patch is adopted.

[Physical results and actual checks](unlocked-input-evidence.md).
