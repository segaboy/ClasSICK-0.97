# Unlocked desktop input trials and helper guard correction

2026-10-26;SRC-0492/0493,IMPL-0078/TEST-0091. Baseline53324a0.

The desktop is unlocked. Capture metadata still does not establish where Windows
injected input lands. In the first isolated PC run the old helper accepted two
+1X events while the host cursor was outside the VM;the guest pixels did not
change. After a held primary-down,Explorer became foreground. The helper refused
further events and reported one owed release;it sent no cleanup to the foreign
window. A fresh observed click on the owned VM titlebar completed a down/up;
physical button-state readback was not available. Do not infer the exact recipient
of the earlier down from a foreground snapshot. The owned trial closes at165.27s,
media/inventory/running states preserved.

SRC-0492 removes the unsafe exception:physical owned-root/client containment is
mandatory. The read-only captured indicator is receipt metadata only. Base guards
are rechecked after its query. Process identity,foreground,Default desktop,primary
mapping and guarded cleanup remain. This fixes host test safety;it does not repair
Mac or PC input or justify more unchanged injected-input retries.

One Copilot Sonnet5.5 Low call returns exact replacements in27.17s from17101 frozen
public/original bytes,one view,zero commands/writes/tests. Codex integrates the
patch,removes a disclosed tautology and checks it. Both the31-assertion native
fixture and independently frozen30-assertion oracle pass and each detects a
compiled restored-bypass defect. The old code fails the frozen oracle. Four helper
CTests pass in Clang Debug,GCC64,GCC32 andASan/UBSan. Initial Python TEMP access
errors are environmental;rerun with the approved outside-Git TEMP/TMP succeeds.
A complete inactive native build/dry-run passes;live refuses before any input.

A separate fresh normal PC trial is offered for the directly connected physical
mouse. No automated input is sent. Its initial screenshot has the cross at center;
150 subsequent guest frames over about65s are identical with the cross at upper
right,counter0000 and unchanged window. Human observation is still unconfirmed:
this does not establish which event caused the jump or prove useful motion/drag.
The [unchanged logical crop](../evidence/screenshots/2026-10-26/pc-corner-pointer.png)
is indexed for the research updater.71PNG/twoGIF;no successful-drag animation.
The300s watchdog closes the VM after301.75s including cleanup:recorded overrun,
diagnostic only,no release qualification. Media/inventory/running states preserved.

The same verified original Mac timer ROM is opened in a fresh Early128K profile,
Relative hardware mode,no disks or shares,no helper input. The last read-only
observation still shows0000 and the original window. No human response is received
before the240s cap;the process closes with ROM unchanged. This is an unconfirmed
check,not a new physical failure. Cleanup and results are in the
[receipt](unlocked-input-evidence.json). No motion,dragging or release pass.

Normal EFI/disk and timer-ROM bytes are unchanged. Prior28612 compiler objects/
six refusals remain unlinked/unrun. No new backend or target execution. Published
source/role/native-app/System1/physical128K/B2criterion6/release gates remain.
All24 research originals,held brief,UART,old rows/tags/releases and retired wiki
are preserved. No Apple/MS/emulator implementation,manual samples,reference
system,new platform,OneNote/wiki,Codex subagent or release/tag publication.

Copilot reports one premium-request cost;all-four remaining balances unavailable.
Other worker qualifications are reused. No provider token/quota equivalence claim.

## Bounded GUI raw-checkpoint follow-up

SRC-0494:two fresh isolated runs reuse the verified raw-motion-later candidate.
One one-pixel Sky drag action includes positioning/button events;it is not an
isolated physical delta. The cross jumps to the corner;window/counter remain.
The first attempt is paused before its known checkpoint. One corrected repeat
polls RIP for15.30s after the action but never sees the required1002D0F2/F4 spin
or verified CSPRAW01 mailbox. No registers are decoded as packet data. No third
repeat. Both VMs close at127.01/104.74s under180s caps,with media/inventory/running
states preserved. No new screenshot is needed for the same corner scene.

The existing profile selects raw-motion-later,whose predicate skips the first
complete packet. This is a documented trigger limit,not proof of which reports
arrived. Before another capture,review/select the existing first-motion profile
or separate a later report. Do not change the decoder or invert signs from an
unqualified snapshot. Five owned sessions are now closed;physical confirmations
and both release input gates remain.

## Repeated physical checks and source review

SRC-0495:the owner withdraws the earlier reports as potentially inaccurate.
Fresh repeats supersede them. PC:owner cannot see the guest cross.118 guest
frames show initial center then upper-right clipping,window/counter unchanged.
This records an unsuccessful usable-pointer check,not a decoded packet or root
cause. No injected mouse events are sent in the physical repeat. Owned PC closes
192.23s under270s cap,media/inventory/running states preserved.

Mac:owner sees the cross staying centered during requested slow right/left and
up/down movement. Its own scene reaches0002,showing button activity;exact physical
press count is not confirmed. This normal timer candidate has no automatic idle
stop. No CPU/phase-record trace is captured. The [unaltered camera export](../evidence/screenshots/2026-10-26/mac-timer-physical-stationary.png) is indexed.
Pause/camera controls are used only after the physical report. The screenshot is
resolved through Windows' actual known Desktop folder after the ordinary Desktop
path is absent. Own Snow closes166.11s,ROM unchanged. No guest shutdown claimed.
All seven owned sessions are closed.72PNG/twoGIF;no successful-drag animation.

SRC-0496:one fresh Copilot original-code review takes41.89s/30812 frozen public
bytes,one view,zero commands/writes/tests,one reported premium-request cost. No
demonstrated defect or proposed patch. Lead retains a narrower diagnostic gap:
observed phase changes,admitted nonzero motion and delivered motion need separate
observables. Reject silent trace-field reuse,button-only early termination,an
unverified timer-frequency inference and any physical-target-hardware claim.
The first500-service idle control was not the physical candidate just tested.
[Lead dispositions and next experiment](mac-stationary-pointer-review.md).

Safety-fix49b619c completes normal CI40426162241 successfully in Windows and Mac.
No further production code or candidate image changed. Remaining release blockers
are usable Mac motion and PC GUI motion/dragging;286 compiler proofs retained,
unrun. Two Copilot calls in this checkpoint report total two premium-request
costs;all provider remaining balances unavailable.
