# Mac first-loss observation

SRC-0424; SPEC-0072 / IMPL-0074 / TEST-0087. Follow-up to the original ROM's
active interrupts but zero accepted pointer samples. One Copilot Sonnet5.5 low
primary supplies original code and tests from a frozen public-only packet.
Codex integrates its artifact, separates hard faults from losses and corrects
two test expectations. No second worker or external retry. Two reads of the
same prompt (one redundant); no other inputs, writes, commands or executed
worker tests.108.83s; one reported premium request, three provider requests.
Account balances unavailable; no cross-provider quota-saving claim.

A28-byte first-loss record captures the already-read RR3, three raw PB/RR0
snapshots, previous phase/button bits and entry inbox phase. Reason distinguishes
missing phase, unstable sampling, pending mismatch and overflow, in existing
decision order. No added hardware transaction or change in sample admission.
It survives recovery/stop and clears on fresh prepare. Hard faults, unstable
WAIT and already-LOSS calls do not create it. Native MWP1 v2 appends the record
after its unchanged100-byte prefix; size128. Separate diagnostic prefix unchanged.
Readers retain existing interrupt exclusion; this is not an atomic/public ABI.

Actual verification:5750 checks/32 phase cases; two targeted CTests pass in
Clang Debug, GCC Release and Clang ASan/UBSan. Four compiled original faulty
controls are rejected: overwrite-first, swapped snapshots, wrong reason and
hard-fault-as-loss. Test proposal is not test execution; lead runs these checks.
Native compilation/trial pending. No successful Mac input/PC GUI drag, release,
physical or System1 claim. Prior286 evidence remains valid and is not restarted.
Full receipt: [JSON](mac-first-loss-evidence.json).


## Native counterexample and pointer-only correction

SRC-0425. Native CI40039752635 succeeds at0f0bf6d, ROM96ae9127 uses23076B,
300B BSS/964B graph/4952B conservative frame bound; source hashes/seal/twins/
12 controls/three exact veneers verified before one isolated original-ROM trial.
A primary GUI click reaches0001 and toggles the pattern; two records delivered,
pointer stays256,171. This is a button event, not working motion or dragging.
The subsequent run enters own fault loop PC00400182/SR2700. Black-box CPU log
identifies odd read00007F27 at own4010F0; our disassembly shows a three-byte
by-value sample copied with word access at an odd stack address (4010EE).
No emulator source or foreign ROM/code inspected. First-loss record remainsNONE
because the fault occurs before recording. Frozen foreground counts are not
live-execution proof; the failing trial and artifact are retained and closed.

Codex changes compact samples to pointer-only passing/return, preserving byte
IO order/admission and adding no reads. Three local profiles5750 checks and four
compiled controls pass again; native rebuild/repeat trial pending. The pre-fault
[product capture](../evidence/screenshots/2026-10-23/mac-window-first-click.png)
is indexed with the fault limitation, not promoted to release evidence.


## Actual producer loss and bounded recovery change

SRC-0426: efbc62f native CI40040817047 passes; ROMcd96fdbf22980B used,
300B BSS/964B graph/4976B conservative bound. Matching sources/seal/twins/
12 controls/three veneers verified. Fresh isolated trial accepts one GUI button
event, stage16/trap1/presses1/fault0/CRC DA5C6E10. No alignment exception.
Pointer stays256,171. Producer first-loss at its own known field is UNSTABLE2,
RR3=9, samples AF4C44/9F4444/AF4C4C, previous01000000, entry READY3.
The strict rejection is correct for these changing bytes. Foreground copy is
stillNONE/last-services3519 while direct IRQ counters reach VBL4846/SCC24685.
Thus foreground diagnostics can be stale even in a paused memory snapshot;
persistent interrupt delivery preventing foreground recovery is the engineering
diagnosis. No emulator source or historical/physical claim. Own trial closed.

One isolated unchanged PC image tests Windows Mouse Keys via documented keypad
selection/hold/release/movement. No confirmed hold or useful movement, counter0;
initial/final RGB differ in box512,381..768,555, so do not claim unchanged pixels
or resolve the earlier4x7 difference. Original Flags62/speed80/acceleration3000/
Right-primary1 restored exactly. VM/media/inventory restored. No PC driver edit,
second blind attempt or E3 promotion. Product PNGs are indexed, not desktop UI.

SRC-0427: one distinct next primary (Copilot Sonnet5.5 low,67.16s, one packet
view, zero writes/commands/tests) supplies original loss-quiescence code/tests.
Master disables once after valid LOSS, allowing foreground discard/resume.
Unstable WAIT remains gated; stable queued fresh baseline enables master last.
Initial arm/terminal faults and sample-admission rules remain. Codex fixes two
expected IO counts for the deliberate added mask write.6295 actual checks/
32 phase cases, three local profiles and four compiled original broken controls
pass. New native build/trial pending; no claimed successful recovery or drag.
Both worker calls report one premium request each; balances unavailable. No
additional model review, source research, native-app work, release or wiki.
