# Mac latch investigation and phase-only experiment

2026-10-26;SPEC-0072/SRC-0468..0472/IMPL-0080/TEST-0093.
Owner directs completion of the paired release. Source63bbccf fullCI40253599502
now succeeds in both jobs. Input/release gates are retained,not waived.

## Conditional-latch investigation
The original fixture models separate live/latched DCD per channel,conditional
reset/retained pending,RR3 masks and independent VIA PB. It is a selected
source-informed engineering model,not an emulator implementation or exact
Macintosh/Snow chip model. Source input is the prior reviewed public facts;
no manuals,foreign code or source searches go to workers. State when disabled
is a minimal fixture choice,not a general hardware claim.

209 checks pass in Clang Debug,GCC x64 Release,GCC i686 and Clang ASan/UBSan.
They include fixture self-checks,stable directional samples,changes before/after
acknowledgement,sticky loss/rebaseline,no replay and bounded stop/hard-fault paths.
ResetIUS does not clear the external latch in the model. Repeated LOSS service
does acknowledge retained pending before the state switch;a fresh scripted edge
is explicitly introduced for the recovery test. Renderer/helper cancellation is
simulated as already complete,not actually invoked by this fixture.

Lead corrects the worker's baseline-inclusive IUS count and one direction
expectation from the pre-read direction rule. Lead separates WR15 latching from
WR1 pending indication and adds a WR1-off latch test;the first lead attempt still
used pending to select RR0 and that test correctly failed,then was fixed. Those
initial failures are not passes. A strict-guard defect plus two faulty fixture
versions (unconditional reset and IUS clearing external pending) are detected.
The unchanged default controller satisfies this fixture;no default fix is inferred.

## Narrow liveness experiment
The fixture exposes a conservative base case:PB4/5 change between reads although
all observed DCD and button values equal the retained state and RR3 is zero.
The base contract declares loss. The optional CLASSICK_MAC_PHASE_IDLE_PROBE=1
instead returns EMPTY after the unchanged IO sequence,only for unstable READY,
have-phase,nonquiesced passes satisfying every endpoint predicate. No record is
admitted or coalesced,no phase/button/queue/first-loss field is updated,and only
the existing services count changes. No extra MMIO is performed. The guard sits
AFTER the existing finite-defer continuation checks. Default macro0 removes it.
Header values other than0/1 refuse;no struct,ABI or RAM field is added.

One worker's endpoint reasoning is adopted only as a conditional observational
argument. Its broader odd/even edge phrasing is not accepted as a physical theorem.
Truthful configured IO,ownership and retained pending remain assumptions. No
claim that all edges are observed or that a later physical edge must be retained
by this particular installed emulator. Recorded native DCD-changing failures
still return loss;this experiment does not claim to solve them or PC GUI input.

The new independent differential runner compares131072 scripted transcripts
against the macro0 reference,with defer both0 and1. Exactly60 phase-only cases
change from LOST to EMPTY;all other examined return values,software state and
IO are identical. Defer continuation is checked separately. This enumerates a
released-button/zero-prior-DCD baseline;it is not a universal physical proof.
The provided source tests additionally cover PB triples,DCD/button discrepancies,
RR3 pending,PRIMING,missing phase,quiescence and a captured-style DCD regression.
Lead fixes the worker's10-read expectation,signed shift and raw-padding copy.
Three compiled guard defects (ignore pending,ignore one DCD,skip defer validation)
are detected. The runner is post-review,not claimed author-blind.

Nine focused CTests pass per four host profiles,including default6295 checks,
defer tests,latch209 and both matrix modes. The first i686 run lacked the explicitly
unbuilt freestanding object;all four objects were rebuilt and final nine-suite
JUnit runs pass. No missing-object refusal is counted as a product defect/pass.
Five invalid native CLI combinations refuse before output/tool invocation.
Native phase-only and phase+defer twins/budget/default-byte checks await new CI;
no new target trial is authorized by the flag itself.

## Bounded attempted trial
The existing verified finite-defer ROMcfa16dc2...b4235,source83f7b6f,was launched
once in the approved isolated Early128K/RelativeHw/no-media profile. Source input
hashes,own CRC and64KiB size were checked first. This was the OLD finite-defer
profile,not the new phase-only experiment. A one-shot parent enforces180 host
seconds and owns the process handle;it is not recurring automation.

Capture was black and the native desktop controller returned GetCursorPos
AccessDenied0x80070005 before input. An already approved inactive PID0 helper
then also reports foreground0/cursorNULL with zero attempted/delivered events
in normal execution context. This confirms unavailable desktop input,not its
specific lock/session cause. No security,login or focus bypass was attempted.
The session was force-closed after66.20s,ROM unchanged;not guest shutdown.
No guest trace,pointer or working input was observed. Runtime status reports
ResetHighestIUS unimplemented;that warning alone does not establish a cause.
The user has been asked to make the desktop available;availability is not assumed.
No new screenshot/GIF is admitted (65PNG/twoGIF remain).

## Candidate and release boundary
Original Mac normal/inverted and PC artifacts from passing63bbccf were retrieved
and reidentified:43 Mac source inputs each,own seals,budgets;18 PC binding inputs,
EFI/disk hashes and writer twins. Normal ROM673a2cca...c4965 remains23096 used/
304BSS/964workspace/4996 conservative stack bound. PC EFIde02e416...e55d0f and
disk66a10526...8b5202 are unchanged. These are preflight pins,not a release freeze.
See the [remaining gates](release2-qualification-checklist.md).

Two bounded fresh Copilot Sonnet5.5 Low calls:302-line latch artifact144.86s from
44990B;focused phase proof/artifact67.08s from22509B. One permitted view each,
zero worker edits,commands or tests;both output caps met. One reported premium
request each;other providers idle,all-four balances unavailable. No cross-provider
quota equivalence or measured saving. Raw captures/failed trials stay outside Git.
286 work is preserved,not restarted or added as a release prerequisite.
[Machine receipt](mac-latch-and-phase-evidence.json).
