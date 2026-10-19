# ADR-0056 - Continuous native outline preview input

Accepted2026-10-19,SRC-0298 / SPEC-0050. Candidate paired-native-preview-1.
The PC prototype retained a static scene after its60-second diagnostic stopped
reading input. The Mac native path already keeps polling. The prior human PC
check reported intermittent clicks; no cause for every miss has been established.

Add an explicit compile-time PC preview profile. It continues input after the
nominal deadline and Escape; the host ends the session. Keep every fault return,
argument check and existing timed diagnostic profile. Space/mouse semantics,
decoder/controller,shared scene and native Mac stay unchanged. This is a narrow
session repair,not an unsupported mouse-decoder rewrite or full desktop.

The real-loop hosted fixture injects inputs after61 seconds and ends by a timer
fault. A legacy-linked negative control must reject the old behavior. New native
PC verification must exercise input beyond60/90/120 seconds and preserve the
scene; the Mac's unchanged exact image/boot evidence is reused. Automated input
clearance does not turn the previous human check into a pass. Hardware,B2,128K,
historical/edition,native external applications and System1 boot goals stay open.
No release/tag/publication permission follows from this decision.
