# Focused input-boundary review

2026-10-25;SRC-0463..0465. Owner requested focused research followed by concrete
development progress. This review reuses the admitted sources and existing
trials;no new VM/emulator/hardware run or physical input is credited.

## Mac finding
Reidentified SRC-0363 (Zilog UM010902-0609):88330952B,SHA256
2a13683ee0e423ee3cdf2781a4ce6b4df65c1e069a269e4234fa6964b4a3bea0,
394pages. Rechecked printed56..59 and177 (PDF page=printed+7),including the
57/59/177 page images. No manual text or page was sent to the worker.
Enabled external status uses channel latches;acknowledging can leave a new
condition pending. The latch status is not an event queue. Family read-latching
differences and independent VIA observations still prevent a coherent physical
snapshot guarantee. No manufacturer's ISR recipe is adopted.

Our service reads two PB/RR0 snapshots before Reset External/Status,then one
afterwards. The retained own trace is AF4C4C,9F4444,9F4C4C;masked XOR between
the first two is0x300808 (PB phases and both DCD bits change). Those reads occur
BEFORE acknowledgement. The later XOR is0x000808. Therefore an acknowledgement-only
change cannot remove the pre-ack instability in this particular record. This
does not identify its host gesture,physical chronology,chip revision,or cause.
Strict loss handling correctly refuses that inconsistent record. No SCC selector,
polarity,spacing,acknowledgement or sample-admission edit is justified here.

The existing host fake clears pending unconditionally on Reset External/Status.
That is an intentionally simple fixture,not the full conditional-latch behavior
of the admitted family description. The next main release investigation is a
bounded original conditional-latch fixture with three separately labeled cases:
stable single-axis capture;change between pre-ack reads;post-ack retained pending
transition. Verify observed loss/cancel/rebaseline behavior without assuming an
edge queue or weakening checks. Such new fixture cases are PLANNED,not run here.
A later live comparison must freeze the idle baseline automatically and observe
one explicitly requested axis gesture without mouse-driven pause contamination.
No physical check or repeated unchanged native trial is requested now.

## PC finding
Oracle's7.2 [user manual](https://docs.oracle.com/en/virtualization/virtualbox/7.2/user/working-with-vms.html)
distinguishes captured PS/2 input from tablet/integration paths. ClasSICK's
current guest uses PS/2;the prior indicator check establishes capture,not guest
acceptance of Windows-injected events. The documentation supplies no guarantee
that would turn the negative injected-input control into a driver diagnosis.
No guest mouse-policy change or new host input API follows from this review.

Snow's current [input guide](https://docs.snowemu.com/manual/input) describes a
hardware-relative mode suited to non-original ROMs and the importance of capture.
Its documented absolute mode alters low memory and remains excluded. Current
capture/menu details are not proof for installed1.5.0-317c188;the prior observed
UI mismatch stays open. No emulator source,installation or upgrade was inspected.
An unrelated Microsoft sample-code search result was excluded;no facts or
implementation from it are adopted. That new research branch stopped.

## Concrete defect repaired
Prior711bf84 CI40245682486:Mac job succeeds,Windows fails in GCC-i686 host
instrumentation compilation. GetTickCount64 and QueryFullProcessImageNameW are
hidden by that SDK's older default API level. The same failure is reproduced
locally before changing CMake. Explicit WINVER/_WIN32_WINNT=0x0A00 definitions on
the two host-only native guard/capture targets fix it. All four helper CTests
then pass in i686. Manual-build guidance names the same definitions. No Windows
API bodies,new functions,OS target requirement or guest code changed.

The chosen independent development slice is the [286 checked-write experiment](pc286-store4-model.md).
It passes real host writes/refusal controls and compile-only C16 closure checks,
with no native-provider or boot claim. Paired release remains blocked by Mac
motion and PC GUI drag/release. [Machine evidence](input-review-and-store4.json).
