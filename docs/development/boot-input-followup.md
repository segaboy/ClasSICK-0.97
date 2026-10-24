# Native boot and input follow-up

2026-10-23; SRC-0416..0418; SPEC-0072/0073, IMPL-0074/0075, TEST-0087/0088.

The accessible-desktop trials find two distinct issues. The Mac ROM draws the
shared scene, then refuses SCC setup. The PC accepts a GUI click when the test
uses the host's primary button; the previous automated drag did not use it.
Release2 remains unfrozen. These observations supersede the earlier UNBOOTED/
desktop-inaccessible checkpoint for this one default Mac and diagnostic PC image.

## Mac observation and bounded correction

The exact ae4ae47 default ROM (SHA256 d1ef824abab8deaacdc1ab10676acb483551559ec724f652dfc9918f478b9f2f)
draws the original512x342 scene in Snow Early128K. Snow's unchanged PNG uses
its own two-colour display palette. Its thresholded pixel mask matches the
PC's centered initial scene exactly; RGB byte parity is not claimed.

Own registers show PC0040072C/SR2700/D000000006. Black-box runtime warnings
report unimplemented reads/writes at the selected lowest SCC aliases. No
counter response or functional input follows. This is a failed input candidate,
not a successful session. No emulator source, reference ROM or disk was read.
Absolute workspace paths correct a separate launch-harness error; failed
relative-path/process attempts are retained outside Git. The owned Snow closed.

The next original engineering profile selects the upper eight-byte cell of
the same published SCC read/write windows, retaining the inferred A/B bit and
read-even/write-odd rule. Only four platform constants and matching manifest
values change; no address scan, fallback, IRQ or loss-policy change. It remains
an unverified board hypothesis pending native compiler and own-ROM trials.
The old rejected image remains negative evidence; no physical mapping or
historical implementation is inferred. Held source material remains excluded.

## PC GUI finding

One fresh isolated boot uses the verified optional probe EFI, SHA256
2ce9b2c1ec39d3e1d0fa1f58aed6ba04cad8639df714d2304098f11ea8734531.
After capture, the old default-button drag leaves counter0000/window unchanged.
RIP10008F65 is our INB return, outside the own probe spin1002D052..1002D055;
no register mailbox is decoded and the loss hypothesis is not demonstrated.

Read-only host preference is SwapMouseButtons=1. An explicit SKY right-button
command activates Snow's menus and, after VirtualBox capture, the native PC
left-button counter advances to0001. RIP after that check is10007447, again
outside the probe spin. The host setting is unchanged in this trial. This
explains the failed click test. The supported drag API has no button selector;
GUI drag with the primary button still needs qualification. No guest driver
change is justified by the previous automated-button mismatch.

The VM powers off and unregisters; exact raw-media roundtrip and existing VM/
running inventory are preserved. An initial unregister hits a transient GUI
session lock; retry follows powered-off verification. No broad memory read,
core dump, firmware inspection or emulator-source inspection occurred.

## Verification and next gate

The receipt records actual image identities, raw observations and closure.
Two original framebuffer PNGs are indexed for the research updater. No new GIF:
no new successful drag sequence has been captured. E1-E3 on both final native
candidates, final package review and separate publication approval remain.
No E4/hardware/System1/full128K claim.286 progress is unchanged, not restarted.

No new external worker is warranted for these bounded observations and four
constant edits. Prior qualified implementations/reviews are reused; no provider
retry or duplicate review. Remaining Claude/Grok/Copilot/Antigravity balances
are unavailable. Native compilation and subsequent trials are reported separately.


## Revised native trial and required-register check

Source4cadc695 passes the native job of CI40033711283. Default ROM SHA256
61b34a6df7692baa8b471797338bee1b26991c7bf46a8df477b76822304183d1;
source hashes/seal/twins/12 controls/three exact veneers verified.22500B ROM,
244B data,964B graph,4912B conservative frame sum. A fresh own-ROM Snow trial
shows the scene and reaches named SCC A/B controls. The model then warns that
WR12 writes/RR12 reads are unimplemented; prepare correctly refuses. Trial closed.

The lead visually checks SRC-0408 Figure10.2 (printed1-17): RR15 bit3 is DCD
interrupt enable, bits0/2 zero, other fields other external interrupt enables.
Existing eligible WR15 DCD-mask facts are reused. No sample code or algorithm.
The original refinement checks distinct A8/B0 DCD masks through RR15, then sets
B8 while MIE remains off. It keeps alias rejection/first fault and has no baud
access, fallback, scan or permissive mode. A missing RR15 capability still fails.

Claude's one19143B bounded original-input call ends after180.81s with a server
error/10 internally reported API retries and no artifact. No external retry or
second worker. Codex supplies the small change and transaction-order oracle.
4549 actual checks/32 phase/inversion cases pass locally; compiled original
fault controls reject sequential write/read alias handling, missing B re-enable
and prematurely enabled master interrupts. GCC/Clang/sanitized checks are local;
new native compiler and boot observations remain separate.

## Normal PC image with corrected primary mapping

A fresh exact normal EFI trial (de02e416, no probe) temporarily selects Left
primary through Windows Settings, then restores Right and verifies readback1.
Other mouse preferences remain unchanged. GUI dragging generates two accepted
press edges, counter0002, but window borders stay at the initial80,70 and the
guest cursor ends511,341. One4x7 cursor-like content difference remains; a
paused publication prefix is possible, persistent damage is not established.
The second gesture is explicitly
preconditioned by E2 cursor-only placement at100,78; subsequent press/move/release
is through Windows. This is still a failed drag, not E3 qualification. The
captured injected-coordinate path remains unresolved; no PC guest driver change.
The native counter crop equals the previously verified counter2 capture exactly.
The VM closes with existing inventory/running states and raw media unchanged.


Sanitizer harness correction: initial launch omitted the pinned compiler-runtime
directory from PATH and stalled/failed without a test result. Attempts are
retained; activating the normal development environment makes both targeted
ASan/UBSan tests pass in0.28s. The test helper was also renamed log_access;
there is no evidence that the former helper name caused the launch failure.
The final test source hash and actual compiled controls are in the receipt.


## Actual RR15 boot result and final diagnostic finding

Native source0ce4c34e, CI40036196626 native job success. The exact65536B ROM
SHA256 dee0760cc4c66287979dfe02f15c31cadabdf962c595de2c7e0db3ab1d33e184
uses22480B ROM/244B data/964B graph. Source hashes/seal/twins/12 corruption
controls/three exact IRQ veneers pass. A fresh original-only Snow trial reaches
diagnostic stage16 with fault0, one original trap probe and matching image CRC.
The rebuilt Mac capture's thresholded monochrome mask equals the PC initial
scene. Palette RGB bytes differ, as disclosed above.

The own100B MWP1 record and adjacent own diagnostic prefix are read via Snow's
public Memory UI, bounded to their known addresses. Initial default display
contains only our own vector table. No memory scan, foreign ROM, disassembly or
emulator source. Live reads across two views are not an atomic/timing snapshot.
Observed: foreground ACTIVE, motion ARM stage, VBL/SCC handlers and service
counts advancing, four baselines, three loss/rebaseline cycles, peak queued0,
deliveries0, pointer256,171, window80,70, cause0, diagnostic presses0/fault0.
Thus controller startup is fixed, but no input sample reaches the owner. Earlier
visual pointer annotations were not proof of guest movement. Mac input is NOT
qualified. Snow also logs unsupported ResetHighestIUS; impact remains unqualified.

One bounded attempt uses the documented Windows Mouse Keys operator interface
for button selection/hold/release. Injected keypad sequences do not change the
Mac counter, so held-button delivery is not proven. The feature/NumLock condition
are restored exactly: Flags62,MaximumSpeed80,TimeToMaximumSpeed3000; original
Right-primary setting1 remains. No security/privacy changes or helper bypass.
The resolved missing-sanitizer-DLL dialogs are dismissed; no reinstall occurs.
Owned Snow and both new PC trials are closed; PC media and existing VM states
are preserved. Four original PNGs from this continuation are indexed; no new GIF.

Next technical work is a bounded source/trace review of Mac sample admission
and real input pacing, plus the captured Windows coordinate path and transient
pixel difference. Keep the strict loss policy until a defect or revised
contract has evidence; no invented successful E3/E4 or release freeze.286 status
is unchanged and its completed work is not restarted. Full RR15 Windows CI is
pending at this record; the preceding4cadc69 full native/Windows run passed.

Final CI update: source0ce4c34e completes both native and Windows jobs successfully
in [run40036196626](https://github.com/segaboy/ClasSICK-0.97/actions/runs/40036196626).
This supersedes pending-source-CI wording above; input/release gates remain open.


2026-10-24 correction,SRC-0431: the earlier4x7 pixel difference is now classified as cursor restoration. A pure-RAM shared-core replay of the observed counter2/pattern0/window80,70/pointer511,341 final state matches every logical pixel of that saved screenshot. No window damage found; the replay is not a reconstruction of its input history and does not qualify GUI drag. [Evidence](pc-first-motion-evidence.md).
