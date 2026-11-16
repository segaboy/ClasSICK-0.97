# TEST-0196 - Buffered acquisition and paired input

2026-11-16; IMPL-0138. Twenty-nine rebuilt existing checks; two 12,132-check
FIFO/deferred-decode profiles; source-matched native twins; both edge-2,048
and edge-4,096 eight-state complete pixel/replay comparisons; actual interrupt
register preservation with deliberate negative comparisons; excessive-burst
overflow, release and fresh-movement recovery. Commands, identities, failed
experiments and limits are in the
[canonical receipt](../../docs/development/paired-preview-readiness-20261116.md).
No physical/human qualification or release substitution.

2026-11-16 human-test follow-up: M1 failed pointer responsiveness and boundary
reach. Both FIFO capacities pass O0/O2 contracts; the experimental 512-entry
image replays the recorded motion at edge 4096 without omissions or decoder/
capture losses, retains eight-state paired pixels, and still detects and recovers
from deliberate excessive input. Detailed identities, failed pacing controls
and the pending M1R1 human retest are in the
[guided human receipt](../../docs/development/guided-human-qualification-20261116.md).

## Sensitivity follow-up, 2026-11-16

Host contracts pass at all four capacities and both optimization levels.
The selected 2048-entry native image retains paired pixels/replay and explicit
excessive-rate recovery, and consumes the M1R2 accepted vectors scaled by
1/2/4 without losses. Smaller-buffer failures remain retained. These are
modeled checks; M1R3 human sensitivity is pending. See the
[guided human receipt](../../docs/development/guided-human-qualification-20261116.md).
