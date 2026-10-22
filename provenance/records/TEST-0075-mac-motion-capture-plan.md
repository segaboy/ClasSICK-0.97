# TEST-0075 - Mac capture/drain planned cases

2026-10-22;SRC-0362..0365;engineering proposal review only.
No stable API/implementation exists here. All cases are UNEXECUTED.
[Dispositions and prerequisites](../../docs/development/mac-motion-prerequisites.md).

|Case|Planned check|Status|
|---|---|---|
|MP1|Quiescent begin admits no record and performs no MMIO.|UNEXECUTED|
|MP2|One accepted normalized X record reaches the existing input owner once.|UNEXECUTED|
|MP3|One accepted normalized Y record reaches the existing input owner once.|UNEXECUTED|
|MP4|Motion/press/motion/release order is preserved;no button sampled from a later record.|UNEXECUTED|
|MP5|Boundary reversal remains two records:atx0,-1then+1 endsx1;coalescedzero would endx0. Include dragged-window clamp path.|UNEXECUTED|
|MP6|Capacity overflow sets sticky detectable loss;unread prefix is discarded and owner cancel occurs once.|UNEXECUTED|
|MP7|Rebaseline with held left stays suppressed;only a fresh released observation permits a later press.|UNEXECUTED|
|MP8|Pre-baseline/stopped records never cross into a new session;generation exhaustion cannot wrap into replay.|UNEXECUTED|
|MP9|Saturated diagnostic counters do not suppress or duplicate valid delivery.|UNEXECUTED|
|MP10|Owner refusal/fault retains its diagnostic prefix;no requeue,auto-repair or leaf-call retry.|UNEXECUTED|
|MP11|Stop at publication boundaries exposes no partial record,ends admission and retains diagnostic state.|UNEXECUTED|
|MP12|Marked uncertainty takes cancellation path;hidden even-edge collapse cannot be claimed detected. Synthetic interleavings are not hardware timing proof.|UNEXECUTED|

MP5 replaces the worker's coalescing expectation;MP7/10/11 use corrected owner
release/fault/stop rules. Synthetic records/interleavings cannot establish
hardware timing,polarity,interrupt safety or a physical no-loss guarantee.
