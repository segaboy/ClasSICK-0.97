<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Mouse correction and release checks, 2026-11-05

Baseline `48064aad6063e06d15bfc00b855e0f716038fbbf` passed both jobs in [CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/41705036533). Paired release remains blocked. This work does not waive the earlier failed human observations, physical qualification or remaining E1-E3 cases.

## Mac correction

The actual collector rejected an otherwise coherent X movement when only the quiet Y axis's direction-phase bit changed (and conversely). A new test failed before correction. SPEC-0096 now allows that unused phase change only in READY with retained state and no quiescence. Pending consistency, all three DCD samples, button stability and the moving axis's direction remain checked. WAIT and PRIMING are unchanged. The change remains confined to the opt-in short collector.

The implementation-aware model matrix covers both axes, all four starting DCD states, all four direction-phase states, four polarity settings, four pending combinations, eight phase/button changes, and two in-pass injection positions: 4,096 cases per profile. Strict and corrected profiles pass 33,570 and 34,082 checks respectively, including hidden-edge controls, under GCC and Clang address/undefined-behavior sanitizers. A deliberately faulty moving-axis relaxation is rejected. Eight focused CTest suites pass. These are original model checks, not physical proofs. The characterization also demonstrates that three completed same-direction DCD edges can look identical to one edge even to the original strict sampler; neither version guarantees arbitrary-rate losslessness. We do not adopt the worker's broader claims about all possible hidden trajectories.

Native twins pass: ROM SHA-256 `3790311804a3c88f7b41dae2c280043eed8faa33ae5e6f8eed90d602353a83dc`, 24,012 used ROM bytes, 964 workspace bytes. Flags: window, fast collection, short collection, WAIT-latch check, loss counts, invert=3. The isolated Snow Early128K session used RelativeHw, no disks/shares and the original image. Guarded upward movements visibly moved the guest cross; a sustained primary-button drag moved the window from x=80 to x=96, followed by leftward pointer movement with the window remaining at x=96. A second downward held drag advanced the counter to 2 but moved the window only five pixels, to y=75. It is not a complete second-drag pass.

The final paused own record shows 127 cumulative deliveries and 21 cumulative losses (13 unstable, eight pending), pointer `(266,80)`, window `(96,75)`. The raw rows below are manually transcribed from the owned guest memory viewer. Counts include positioning and pauses; there was no clean phase-isolated loss baseline. API acceptance is not guest acceptance; the helper did not verify exclusive capture. One movement attempt was refused for foreign foreground before sending anything; a retry followed fresh owned-window selection. The owned process was externally stopped and its ROM hash remained unchanged, with no guest-shutdown claim. This confirms bounded motion/drag, not reliable input or a release pass.

```text
00000BD0 00 00 00 00 00 00 00 0D 00 00 00 08 00 00 00 00
00000BE0 4D 57 50 31 00 00 00 02 00 00 00 80 00 00 00 01
00000C00 00 00 00 15 00 00 00 15 00 00 00 00 00 00 00 0B
00000C10 00 00 00 7F 00 00 01 0A 00 00 00 50 00 00 00 60
00000C20 00 00 00 4B 00 00 00 00 00 00 00 01 00 00 00 05
```

Two earlier repeats of the unmodified short candidate did not produce a qualified drag: the first expired before the input helper was bound; the second sent 40 guarded upward events but did not capture a diagnostic result. Both closed under their watchdogs. Some UI calls used an incorrect button parameter and were not credited as successful control actions; the later trial used the documented `mouse_button` parameter and observed each result. No full-window capture is admitted as product media.

The normal fast/invert-3 native twin rebuild retained SHA-256 `920d90bd16b204d4b159356f274273fff02c00f4a960c0cd78880c7ffb7bba1e`; the correction is absent with SHORT disabled. The SPEC-0096 source-locator correction was made after the experimental build; it does not change compiled inputs. Artifacts: `C:\ClasSICK\mac-axis-phase-20261105-a`; earlier attempts: `C:\ClasSICK\mac-short-repeat-20261105-a` and `-b`.

## PC scaling control

SRC-0593: Qt's published [High DPI configuration table](https://doc.qt.io/qt-6/highdpi.html) documents `QT_ENABLE_HIGHDPI_SCALING=0`; the installed VBoxManage 7.2.16 public usage documents `startvm --putenv`. Only these interface facts were used, no implementation source. Search excerpts also returned unrelated Qt declarations; those results were excluded, no source page opened and nothing from them reused or sent to the worker. The environment override was limited to one owned VM process. Whether this frontend actually honored it is unverified; no host-wide display setting changed.

The original disk SHA-256 `94bba3d46dac625a7d2b4a1ae58d0e775326253191503710f4b9bb00f82f35b1` and EFI `8e871da4e540da3f09498e3729dce55d92c9f6f95ecec464dcf6b778f0b55435` were unchanged. Networking, sharing, clipboard, drag-and-drop, USB, audio and UART remained disabled. A sustained primary click yielded a verified capture indicator, but Windows pointer position again became `(2,2)`, outside the original window. After guarded placement of only the owned window around that point, 32 requested `(-8,-8)` movements were accepted by Windows; later checks showed `(0,0)`.

The final guest frame did not reach the intended top-left. Its decoded RGB is pixel-identical to the already registered [failed corner frame](../evidence/screenshots/2026-11-03/pc-host-corner-attempt.png); changed pixels relative to this trial's initial frame lie only in `(503,375)-(520,392)`, exclusive right/bottom. This configuration did not fix the observed problem. The original final PNG SHA-256 is `ab670643740f705c0b2449ce5432d65a286473111dc9d49090b1337fda551be6`; no duplicate picture is added to Git.

The VM was externally powered off and unregistered; existing inventory/running states and the post-trial raw disk roundtrip match. Total trial host time was 312.17 seconds, including preparation and paused waits. Initial binding attempts refused before input (multiple launcher processes, then inactive generated binding); the corrected helper bound the process owning the returned window. The receipts are at `C:\ClasSICK\pc-scale-trial-20261105-a`. No PC driver fix or E3/E4 pass follows.

## Worker review

Claude `claude-opus-5-5`, explicit Low, reviewed a frozen 40,435-byte original public-source packet, SHA-256 `26a1d009e6ca86de34b67b8bd13bc2f9453e2376cfd4714d38b9248e91452ac7`. It returned in 111.25 seconds with no tools or executed tests. The lead reproduced the quiet-axis defect and integrated its minimal acceptance rule; the diagnostic-field expansion was not adopted. Provider usage: two other input tokens, 23,426 cache-creation tokens, 1,452 cache-read tokens, 10,797 output tokens; $0.4036464 list-basis estimate, not billing or Codex savings. Remaining allowance was not reported. Raw packet/transcript: `C:\ClasSICK\release2-completion-20261023-a\mac-sampling-review-20261105\claude`.

## Late-edge race, diagnostic masking and motion omission

Follow-up from `d22c7fb4db1bc32dd7583fc444f1b4f5b1726ab3`, whose Windows/Linux CI passed. SRC-0594 / SPEC-0096 / IMPL-0111 / TEST-0158. Three changes are confined to the opt-in short collector:

- A clean edge arriving after the entry RR3 read can now remain pending for the next finite pass. The new model regression failed before the correction. No movement is inferred or emitted on the retry; invalid inbox metadata cannot take this shortcut.
- Foreground diagnostics no longer repeatedly mask interrupts to copy unchanged data. Service changes and non-empty foreground operations request a coherent copy under the existing exclusion. Saturation copies every time. Native inspection confirms single longword reads before exclusion and an in-exclusion generation reread. Worst-case masking duration and physical timing are not measured.
- **Deliberate engineering policy change:** ambiguous displacement with three unchanged button samples may be omitted while retaining the button and drag lifetime. Separate `motion_drops`/MMD1 counts omitted passes. This can lose distance; it does not recover hidden steps. Button ambiguity, missing phase, PRIMING, invalid inbox, queue overflow and explicit capture loss retain cancellation. The late-edge retry precedes omission. Default strict behavior is unchanged.

The final source-informed model passes 40,222 checks in the short profile and 34,594 in the strict profile under GCC and Clang address/undefined-behavior sanitizers. The checks include 4,096 matrix cases per profile, late-edge regression, omitted-motion sequences, invalid bounds/claims, saturation, overflow and hidden-edge limitations. An added whole-software-graph test drives the original controller, inbox, foreground, capture, window owner and renderer: a held drag survives omitted motion on both axes, coherent motion moves the window, release occurs once, later motion leaves the window stationary, and an explicit inbox loss still cancels a new drag. These are serialized hosted model checks, not native GUI observations. Eight focused CTest suites pass. Worker tests were proposals until these lead executions.

Native twins and seal/veneer/stack checks pass. Final experimental ROM SHA-256 `a89056625390a3dc69146dda934c9cacdaeda5d00ca882b68cd71f54bc09b60a`: 24,488 used ROM bytes, 436 data/BSS bytes, 964 workspace bytes and conservative 5,008-byte stack bound. The normal fast/invert-3 twin remains byte-identical at `920d90bd16b204d4b159356f274273fff02c00f4a960c0cd78880c7ffb7bba1e`. No physical qualification follows.

The intermediate race/masking-only ROM (`88c5dae9eafb321738dce74644ca12ca3aa363eeac322e183e8e9b8e9bc54332`) booted in isolated Snow and moved upward. The horizontal drag did not move the frame; the vertical attempt moved it seven pixels. Its final record showed seven hard losses (three unstable, three pending, one overflow), 129 deliveries, peak queue 16, pointer `(278,84)` and window `(80,77)`. Pending-disagreement counts were A pending/no-change=2 and B pending/no-change=3; both change/no-pending counts were zero. Counts include setup and UI; this is not a controlled quantitative comparison with the earlier trial.

The final motion-omission ROM booted in a separate isolated Snow session. One 48-event upward plan was accepted and the guest cross moved upward into the content area beneath the title bar. A foreground refusal sent no input; subsequent control calls repeatedly reported concurrent user input. The session was closed without completing the drag check. No live drag pass is claimed for this image, and old human failures remain. Generated-helper binding errors occurred before input and were corrected against the actual launcher receipt. Both owned Snow processes closed externally with their original image files unchanged. Artifacts: `C:\ClasSICK\mac-late-edge-20261105-a` and `C:\ClasSICK\mac-motion-drop-20261105-a`. No new public image or animation is admitted.

## PC capture-context discrimination

A fresh unchanged PC disk/EFI trial used the approved read-only Windows context caller. After click capture, the indicator said captured and both logical/physical cursor queries returned `(2,2)`, while GetClipCursor still described the full virtual desktop `(0,-1440)-(5200,1440)`. The GUI-thread capture query reported no capture handle. Thus this observation does not support a tiny ClipCursor rectangle as the cause, and the indicator alone still does not prove exclusive delivery. After the standard Right Ctrl host key, the cursor was `(1280,720)` and the rectangle was `(0,-1440)-(4000,1440)`; the indicator still said captured. A subsequent relative-input plan correctly refused physical-client containment before sending any events. The exact cause of the coordinate/rectangle changes is unresolved; no guest-driver fix or successful keyboard-capture transition is inferred.

Only the owned VM was positioned. It closed externally and was unregistered after 295.06 host seconds; prior inventory/running states and disk roundtrip were preserved. Artifacts: `C:\ClasSICK\pc-context-20261105-b`. No repeated corner test or new E3/E4 pass. Oracle's published host-key/capture interface documentation was read; no emulator implementation was used.

## Follow-up worker accounting and next discriminating check

Two bounded Claude Opus5.5 Low calls used frozen original-public inputs, no tools or executed tests. Critical-latency review: 68,951 transmitted bytes, SHA-256 `46347ff6d0532630383342719f37c4369a51851384a38ed03381a9e1f2409633`, 116.09 seconds; 2 input +36,616 cache creation +1,452 cache read +10,562 output tokens, $0.5044664 list estimate. Motion-omission proposal: 53,951 bytes, SHA-256 `4e3b54f18f3bcdebcb9629edb21ad4db0e526210b45083202f8cac4090389f32`, 136.20 seconds; 2 input +30,904 cache creation +1,452 cache read +15,548 output tokens, $0.5584904 list estimate. Estimates are not bills or Codex quota savings. Remaining allowances for all four external services are unreported; Grok, Copilot and Antigravity were not invoked. Raw transcripts remain outside Git under `C:\ClasSICK\release2-completion-20261023-a`.

Lead changes to the proposals: explicit startup initialization, reuse of the existing SHORT gate rather than another flag, native omitted-motion diagnostic, complete inbox eligibility on the late retry, and the whole-graph drag/release/cancel regression. Unmeasured worker cycle estimates and broad safety claims were not adopted. Next: finish the guarded live drag/release on this exact motion-omission image and read MMD1 separately from hard losses. PC frontend capture remains a distinct unresolved blocker. No release criteria, physical or human gate has been waived; release2 is not ready and no release was published.

## Foreground backlog and native rendering

SRC-0596 / IMPL-0112 / TEST-0159; baseline718223194f5a575708cde04e153615a86525e8d3 passed both jobs in CI42087041585. Paired release remains unqualified.

The repeated a8905662 candidate moved the frame from(80,70) to(87,77), but recorded five hard losses, all overflow, and174 omitted passes. Its manually read deliveries field appeared zero despite visible updates; that transcription remains unresolved and is not used as proof of zero deliveries. Artifacts: C:/ClasSICK/mac-motion-drop-20261105-b. MFL1 was then added to distinguish foreground backlog from source ambiguity. It measures counter spans around nonempty foreground steps; these are separate volatile reads, paused observations only, not CPU-cycle or physical timing guarantees. Producer samples are push attempts, not interrupt services.

The diagnostic-only ROM013063a096894ed314a824f4cae3129013305d18a043cc28e82916b75c3f47fa recorded109 nonempty completions,105 DELIVERED, maximum70 VBL ticks and maximum four producer attempts within a delivered step. It recorded two overflow losses and66 omitted motion passes; one guarded horizontal drag moved only seven pixels. Setup/counter rendering and UI interactions are included. These runs are not matched benchmarks. Artifacts: C:/ClasSICK/mac-foreground-latency-20261105-a.

Three original optimizations now retain the same image/error contracts: clipped mono content is written as whole pattern bytes, fills advance row pointers without repeated multiplication, and glyph/number work outside the changed band is skipped. The final additional fast path copies the fixed original scene from read-only ROM, then updates the counter. Its exact gate is MONO1_MSB,width512,height342,physical width512,band within342 rows,value97,progress16,phases6,success1,pixelsNULL; all other callers retain the generic painter. No input policy or queue capacity was changed.

The 21,888-byte artwork is generated solely by the original product renderer and surface code at7182231, state(97,0,16,6,1,NULL). Raw payload SHA-2565ac7431183aee9cd556e850acfcd0139f5d40192c9cadfa96ff473fe20aac9db. Original generator, archived source copies and payload are outside Git at C:/ClasSICK/product-rom-cache-20261105-a; the portable text initializer is apps/product-scene/fixed-mono.h. No Apple/reference artwork, ROM bytes or manual sample was used. The cache costs ROM/image bytes, not a second RAM framebuffer. Builders identify the header. Generic-height343 comparisons independently exercise the uncached painter with the same origin and visible342 rows.

Executed final hosted checks:17 focused suites each under GCC and Clang address/undefined-behavior sanitizers, all pass. The window oracle compares10,040 full frames per profile, including widths2..17, masks, byte phases, clipped bands, both patterns and padding. Product bands cover glyph/number boundaries;1,368 cache comparisons span all342 starts and four counter values. These are hosted equivalence tests, not target timing or usability passes.

The first optimized ROM21e98d82c519379ae4803998f8747e063ff7234678f68ef8d527280a9522ac69 used25,096 ROM bytes and observed maximum42 VBL ticks, eight arrivals,57 deliveries and zero hard losses at an intermediate checkpoint. Several foreground refusals sent no events; one48-event plan left the guest viewport and is not a qualified guest stimulus. A later upward plan moved the guest; the following reverse attempt was incomplete. The session closed under its600-second watchdog. Artifacts: C:/ClasSICK/mac-fast-content-20261105-a.

The band-skipping ROMd7f56afa3924b0dedf5425f39450a9ce63ec91d387f72c135fd55821fa33dc3a used25,276 ROM bytes. A horizontal drag moved x80 to102 and released motion left the frame there; the second vertical attempt moved the pointer but not the frame. Final MFL1:159 nonempty,151 deliveries,max16 ticks,max2 arrivals. MLC1:one unstable,one pending,two overflow; omitted164. Thus that candidate did not fix the blocker. Artifacts: C:/ClasSICK/mac-fast-content-20261105-b.

The final ROM-cache candidate b0e8537157b4167d0b9b5a49a932a537ce1546999028c8dae18cc6835b978a57 passes native twins,seals,veneers and stack checks:47,404 used ROM bytes,476 data/BSS bytes,964 workspace bytes,5,036 conservative stack bound. In isolated Snow Early128K/RelativeHw, a guarded horizontal drag moved the window from(80,70) to(104,70); released leftward motion left it there. A new vertical drag moved it to(104,92); released upward motion left it stationary. Final counter2. These are bounded synthetic host observations, not human or physical passes. Final MFL1:191 nonempty,188 deliveries,max11 ticks,max2 arrivals. MLC1 still recorded one overflow, other reasons zero;85 passes were omitted. Counts include setup and pause interactions, so the remaining overflow is not assigned to a particular phase. Complete loss-free/corner/repeated qualification is still required. Artifacts: C:/ClasSICK/mac-rom-cache-20261105-a.

Raw final paused words (manual transcription, own RAM):

```text
BB0 4D464C31 000000BF 00000003 00004DB3
BC0 00004DB5 0000000B 000000CD 000000CD
BD0 00000002 000000BC 4D4C5831 00000000
BE0 00000000 00000000 00000000 4D4C5031
C00 4D4C4331 00000000 00000000 00000000
C10 00000001 4D4D4431 00000055 4D575031
C20 00000002 00000080 00000001 00005CFC
C30 00000176 00000000 00005E10 00000002
```

Final native-PC cache build passes four compiler twins,27 image corruptions and26 omitted-object rejections. EFI SHA-256df284097539ed6b7c90700cb27589900b20d2e4c7574b7637ba5f387abba9fe9,207,360 bytes. Matching original disk writers and independent media verification pass. This new PC image has not yet booted. The first twin-writer invocation lacked the sanitizer runtime on PATH; it failed before writing its twin. The existing first file was preserved under a new scratch name, then both writers succeeded with the proper tool PATH. No compatibility or boot claim from that build.

Four bounded Claude Opus5.5 Low calls used frozen original public input only, no tools or executed tests. A first bitmap-run proposal was tested then parked outside Git because the Mac uses pixelsNULL; it is not included. A latency proposal was narrowed to actual push-attempt counts rather than treating service counts as arrivals. Byte-pattern and band-skipping patches were integrated; lead added narrow-window and cache tests and moved the value modulus inside its band guard. Input byte/hash, wait seconds and provider counts:

|Call|Bytes / SHA-256|Seconds|Input/cache-create/cache-read/output|List estimate|
|---|---|---:|---|---:|
|parked bitmap runs|16184 /1199087263650d85b44b40e4568dfdb0e4e74e243213bd532ab498768ead6a0d|45.44|2/8725/2361/4735|$0.1649802|
|foreground diagnosis|85945 /41b6e3763cd33c2e588a8b5192bc58d7b07c3f6c44ec6dc8bf372cb6c1f63580|67.08|2/46271/2361/6395|$0.4985482|
|mono content bytes|37166 /569bf4a696fe0411bb034bc0808617dc106d3602491f85b93ed9061f72ffe2af|84.42|2/19498/2361/9009|$0.3366442|
|off-band work|16413 /c999ca74bfb74780605f1977e63fed7dc0c35eebae2d653c4b0feb3728fb40ae|83.92|2/8890/2361/9427|$0.2601402|

Raw packets/transcripts stay under C:/ClasSICK/release2-completion-20261023-a. List estimates are not bills or Codex quota savings; remaining allowances for all external services are unreported. No Grok,Copilot or Antigravity call in this slice.

## PC legacy-mouse observations

SRC-0597. Two isolated runs of the unchanged94bba3d4 disk and8e871da4 EFI used process-local QT_QPA_PLATFORM=windows:nowmpointer. No global display preference changed. The first run's baseline preceded guarded window placement, so its movement cannot isolate placement effects. The second captured its baseline after placement, then sent twelve guarded host events per direction, without intervening UI actions. An exact15x15 original cross/halo matcher found framebuffer centers: baseline(512,384), negativeX(510,384), positiveX(555,384), negativeY(555,343), positiveY(555,367). Correct-sign motion is observed, but negativeX is only two pixels and host coordinates remained near the virtual-screen origin. No corner, drag, exclusive-capture or E4 pass follows. Both VMs were externally closed/unregistered with existing inventory/running states and raw disk roundtrip preserved. Artifacts: C:/ClasSICK/pc-legacy-mouse-20261105-a and -b. A distinct process-local legacy-plus-DPI comparison is prepared separately; its outcome is not implied here.
