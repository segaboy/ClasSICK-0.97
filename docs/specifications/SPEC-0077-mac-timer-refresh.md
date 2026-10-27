# SPEC-0077 - Opt-in timer SCC status refresh

Original engineering experiment,2026-10-27;SRC-0497..0502. Extends SPEC-0075,
not the default Mac controller. No new interface facts:reuse the admitted WR15
DCD latch and WR0 ResetExternalStatus facts in SPEC-0072/SRC-0363. No emulator
implementation,manual samples,Apple/MS code or historical behavior is a source.

CLASSICK_MAC_TIMER_REFRESH_PROBE defaults0;values other than0/1 refuse compilation.
At1,prepare retains the existing alias/readback test,then writes WR15 DCD enable8
to A then B and verifies both8. WR1 and SCC master remain disabled. After an
actual T1 clear and admitted WAIT/PRIMING/READY phase,service sends WR0 0x10 to
A then B once each before the first PB/RR0 triple. Empty-T1,LOSS skip,invalid
lifecycle/epoch,prepare refusals and unexpected inbox-state paths do not refresh.

All existing double-triple coherence checks,UNSTABLE-before-overrun precedence,
Gray loss,whole-input uncertainty,queue/epoch policy,stop/fault behavior,reload4096,
invert and direction mapping remain. There is no new state,callback,API or record
layout. A fault retains existing T1/SCC-master-off cleanup;stop clears both WR15.
Default0 must preserve optimized code and both baseline native ROMs.

Build-LoadedAppROM --window-timer-refresh-probe requires --window-timer-poll-probe
and the existing window profile. Existing mutually exclusive SCC experiments
remain refused. Normal and optional idle refresh images get a distinct profile
and explicit manifest marker. Native twins,seals,source hashes,IRQ closure,RAM/
stack budgets and normal-byte identity precede a target trial.

This endpoint strategy may miss complete/three-step cycles or mixed-chip changes.
Refreshing latches before reads does not establish physical coherence or an edge
count. No physical Macintosh,clock/latency,System1 or release qualification is
implied. The hypothesis is unproven until an actual target observation. Reuse
existing MWP1/MWR1 readout;do not add or silently overload trace fields.
