# Owned-window input qualification - 2026-10-27

SRC-0504..0507;IMPL-0078/TEST-0091. Windows and native Mac CI40569640275 both
pass for3fab75ff8afdc09493684705dde94ebf5b8a1d9f. Guest images are reused unchanged.
The capture route now displays the intended Mac/PC scene on primary Display1;
GLKVM on another display is left alone. Capture reliability does not establish
guest input. **The paired release remains unqualified.**

## Host helper and verification

An explicit physical-left selector retains default Windows-primary semantics.
Positioning-only mode moves the verified owned window to(100,100),1024x854 and
sends no input or foreground request. Process/path/hash/creation,window ownership,
Default desktop,mapping and held-button checks apply. Actual input still requires
foreground and physical WindowFromPoint/root/client containment on every event.
Read-only capture metadata never bypasses those guards. An optional exact window
ID in the outside-Git binding separates a fullscreen canvas from its toolbar.

Published Microsoft MoveWindow declaration/parameter/return facts only are used;
no examples or implementation bodies. This is host instrumentation,not OS or
Mac historical evidence.83 fake native assertions and four helper CTests pass
in each of Clang Debug,GCC64,GCC32 and ASan/UBSan. Five compiled defects are
detected;16 placement dry/inactive/refusal CLI controls pass with zero real input.
Proposed worker tests are not credited;Codex executes these checks.

One Copilot Sonnet5.5 Low primary returns15 replacements in59.56s from23749 frozen
reviewed-public/original bytes,one view,no writes/commands/tests. Lead corrects
the weak held-button assertion,success reason and post-move identity checks;
later adapts positioning to the owner's other-display instruction and binds
the exact observed fullscreen root. One reported premium-request cost/two
provider requests;all four remaining balances unavailable,no measured Codex
savings. Claude,Grok,Argon idle;no repeated qualification or all-tool fan-out.

## Actual Mac result

The unchanged source-verified refresh ROM a5fca848..c3a9 boots Early128K/RelativeHw.
Eight guarded relative +1X events and physical-left held down/up are accepted by
Windows. The actual original scene changes to stripes/counter0001. After pausing,
three overlapping reads of our own RAM record confirm MWP1 v2/128B at BA8:
19839 services,two baselines,two losses,two deliveries,pointer256,171,window80,70.
The diagnostic fault field is0. First loss6 is **a two-bit Gray phase jump**,not
timer overrun5;both sampled endpoints are AF4C4C. Current trace lacks the complete
previous quadrature endpoint. Earlier UI actions can contribute activity,so this
is not a quiet pre-input control or causal attribution of that loss to a gesture.
Button acceptance is observed;motion and dragging do not pass.

The [unaltered Mac export](../evidence/screenshots/2026-10-27/mac-refresh-held-click.png)
and [original record bytes](../evidence/traces/2026-10-27/mac-refresh-paused.json)
are retained. No firmware,reference pixels or guest writes enter the observation.

## Actual PC result

Fixed normal EFI e634d1a4..b955/disk fa627d57..85d0 boots its original1024x768
framebuffer with the centered512x342 scene. The fullscreen test stays on Display1.
All strict foreground/physical-root/client guards pass for eight relative +1X
events,then held physical-left and Windows-primary down/up pairs. Windows accepts
all12 mouse events;the six subsequent guest exports remain pixel-identical to
the baseline. Counter0000/window unchanged. A host Space event is attempted but
is not interpreted as a keyboard test for this window profile. Capture acceptance
inside the emulator is unconfirmed;host API success is not guest acceptance.

The [unaltered PC export](../evidence/screenshots/2026-10-27/pc-fullscreen-input-unchanged.png)
records the negative result. No drag is attempted from an unverified pointer
position. Existing virtual-device E2 positives do not become GUI E3 evidence.

## Trial accounting and next discriminating work

Thirteen owned launch attempts are closed (three Mac,ten PC),including startup,
placement and fullscreen/readiness diagnostics. None becomes a release pass.
The receipt retains each cleanup/cap result and initial failures:one Mac trial
overruns240s by0.3144s,one fullscreen notice outlives its old runner,one readiness
check pauses before video starts,and one transient startup-lock failure aborts.
Corrections wait for running state and the actual original scene;the final PC
closes190.29s under390s,Mac178.53s under360s. Prior registered/running VMs and
all original media are preserved. GLKVM is not controlled. No sessions remain.

Do not repeat unchanged injected gestures or infer an SCC/sign/decoder fix.
Next Mac evidence needs separate raw phase activity,admitted nonzero motion and
delivery observations,including the previous PB endpoint,as already proposed
in the stationary-pointer review. The PC needs a confirmed guest-accepted host
route or a directly connected physical-mouse observation on Display1;the pending
availability question has no answer and a human test is not claimed. Retain all
E1/E2/E3/E4 distinctions and failed usability observations. Freeze/package approval
waits for real movement/drag/release/reentry/cancel/restoration on exact candidates.

74PNG/twoGIF are indexed;no successful-drag GIF. The286 compiler proofs remain
unlinked/unrun with no new286 work here. All24 originals,held brief,UART,tags and
old records are preserved. No new source exposure,reference/hardware/System1
claim,native-application work,release publication,OneNote/wiki or Codex subagent.
Raw transcripts stay outside Git. [Machine receipt](owned-window-input-evidence.json).
