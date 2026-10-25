# ClasSICK 0.97 working instructions

2026-10-25,SRC-0440/0441:owner chooses independent work while using desktop. Original paused-clock candidate tool passes49 CLI cases/14 preserved legacy controls;portable Windows/Linux CI added. Native timing trial deferred,no input result;own process/ROM/preference preserved. Prior full CI passes,new CI pending.63PNG/twoGIF;motion/GUI/release gates open,286 unchanged. [Evidence](docs/development/paused-clock-evidence.md).

2026-10-24,SRC-0438/0439:original replay generator passes14 checks;unchanged-ROM one-event trial later retains X-only UNSTABLE/RR3zero,fault0/no displacement. Playback timing/pre-load GUI not isolated,no causal fix justified. Both prior source CI jobs pass;owned sessions/ROMs/preference preserved.63PNG/twoGIF,no new visible motion. Mac motion/PC GUI/release gates remain;286 unchanged. [Evidence](docs/development/mac-input-replay-evidence.md).

2026-10-24,SRC-0436/0437:automatic idle diagnostic reaches own PC004002DC/IPL7 after500 services,baseline1/loss0/delivery0/fault0 before deliberate GUI input. Normal ROM byte-identical;probe adds16B ROM/zero RAM,13 native controls/three IRQ veneers/five refusals pass.63PNG/twoGIF,own trial closed/preserved. Native source CI passes,Windows pending;Mac motion/PC GUI/release gates remain,286 unchanged. [Evidence](docs/development/mac-idle-probe-evidence.md).

2026-10-24,SRC-0434/0435:Mac audit finds no supported code fix. Reused verified
ROM accepts1press/2records/fault0,pointer fixed;losses1->3,pre-gesture control
not clean idle. Normalized pixels match;62PNG/twoGIF. Own process/host preference
preserved;full source CI40098551352 passes. Runtime unchanged,Mac motion/PC GUI
release gates remain;286 unchanged. [Audit](docs/development/mac-motion-audit-evidence.md).

2026-10-24,SRC-0433: later PC packet3 again dx255/dy-255/right button2,
not a reliable small relative left-drag route. Stop unchanged SKY drag retries.
Normal binary unchanged;201checks/three profiles/native audits pass.61PNG/twoGIF,
VM/media/inventory preserved. Mac motion/PC GUI/release gates open;286 unchanged.
[Current evidence](docs/development/pc-following-motion-evidence.md).

2026-10-24,SRC-0432: optional motion-later diagnostic waits for total packet
count>=2;201 checks/three profiles/three compiled defects/seven mode refusals
pass, normal EFI unchanged. Committed-source native later-packet build/trial
pending; no input-policy/release change. [Evidence](docs/development/pc-following-motion-evidence.md).

2026-10-24,SRC-0431: first-motion source999f5bb7 full CI40094774719 passes.
Actual guarded packet for one-pixel GUI gesture is dx255/dy-255/right button2;
native clipping correct, no left drag. Header/X are cleared parser state,
not retained raw bytes. Exact pixel replay clears prior4x7 damage suspicion.
58 PNGs/two GIFs; owned VM closed/media/inventory preserved. Normal image
unchanged; Mac motion and PC GUI drag still block release. Older pending
entries below are historical. [Evidence](docs/development/pc-first-motion-evidence.md).

2026-10-24,SRC-0429/0430: priorb3cb3fe full CI40042117791 passes. Optional
first-motion PC probe passes48 checks/three profiles/three compiled defects/
eight mode refusals; normal EFI byte-identical. Source-committed diagnostic
trial pending, no input-policy change or release pass. Preserve originals and
all gates. [Receipt](docs/development/pc-first-motion-evidence.md).

2026-10-23,SRC-0428: nativeb3cb3fe loss-gating trial shows foreground recovery,
two baselines/two losses, fault0, but zero samples/presses and fixed pointer.
6295 checks/three local profiles/four compiled recovery controls pass; native
CI40042117791 passes,Windows pending.55 PNG/two GIF; sessions/settings/media/
originals preserved. Mac motion/PC GUI drag/release remain blocked.
[Receipt](docs/development/mac-first-loss-evidence.md).

2026-10-23,SRC-0426/0427: nativeefbc62f accepts click0001/fault0; actual
producer loss is UNSTABLE, foreground copy stale while IRQs advance. Original
SCC loss-quiescence/rebaseline rearm passes6295 local checks/three profiles/
four compiled controls; native repeat pending. PC MouseKeys negative/restored;
54 PNG/two GIF. No drag/release pass. [Receipt](docs/development/mac-first-loss-evidence.md).

2026-10-23,SRC-0425: native0f0bf6d trial accepts GUI click0001/pattern toggle,
then its new three-byte by-value diagnostic copy faults at an odd stack address.
Lead pointer-only correction passes local profiles; native repeat pending.
Motion/drag/release gates remain open;51 PNG/two GIF. [Receipt](docs/development/mac-first-loss-evidence.md).

2026-10-23,SRC-0424: first-loss observation only, no sample-admission/MMIO change.
5750 checks/32 phase cases/three local profiles/four compiled controls pass.
MWP1 v2 size128/prefix100; native build and trial pending. Mac input and PC GUI
drag/release gates remain open. [Receipt](docs/development/mac-first-loss-evidence.md).

2026-10-23 final boot/input follow-up,SRC-0422/0423: RR15 source0ce4c34e
native-verifies and boots to stage16/fault0 in Snow. Own trace has live VBL/SCC,
four baselines/three losses, zero delivered samples; Mac input NOT qualified.
PC GUI counter0002 works, drag still fails and one4x7 snapshot difference is
unclassified. Original primary/MouseKeys settings restored exactly; all owned
sessions closed, media/VM inventory preserved.50 original PNGs/two existing GIFs.
No release freeze/E3/E4/hardware/System1 claim. Full native/Windows source
CI40036196626 passes at0ce4c34e. [Receipt](docs/development/boot-input-followup.json).

2026-10-23,SRC-0419..0421: upper SCC aliases reach named controls in Snow;
its unused WR12/RR12 model is unimplemented, so a DCD-mask WR15/RR15 guard
replaces baud markers without dropping alias checks.4549 synthetic checks/
32 phase cases/three compiled faulty controls pass; new native compile/trial
pending. Normal PC GUI counter reaches0002 with standard test primary mapping,
drag still fails; original Right-primary host preference restored/read back.
One4x7 snapshot difference remains unclassified. No PC driver-policy change,
release freeze, new source waiver or physical/reference claim. [Follow-up](docs/development/boot-input-followup.json).

2026-10-23 follow-up,SRC-0416..0418: desktop trials resumed. Mac default ROM
draws the shared scene then refuses SCC setup at unimplemented lowest aliases.
Revised upper-cell alias profile remains an unverified engineering candidate.
PC diagnostic GUI primary click advances0001 with swapped host button mapping;
the old default-button drag did not exercise primary dragging. No observed
LOST spin/mailbox, no PC loss-policy change. Both owned trials closed/media/
VM inventory preserved. E3 drag/Mac input/release gates remain. [Receipt](docs/development/boot-input-followup.json).

2026-10-23 current checkpoint: native window ROM/compiler audit passes atae4ae47
(two declared polarity profiles,22500B ROM/964B workspace), both UNBOOTED.
PC device-control drag passes, automated GUI drag still FAILED. Later Windows
GUI access is denied, so the diagnostic VM was closed before input; no mailbox
result.15 original PNGs/new exact-identity GIF recorded.286 model7bb9856
verified, no far backend/target run. Release2 is NOT ready/frozen. Explicit
three-scope approval below remains valid for needed verified original trials;
publication/reference/physical hardware separate. [Receipt](docs/development/release2-integration-evidence.json).

2026-10-23,SRC-0402..0411/SPEC-0070..0072: explicit owner three-scope
approval clears original SCC/VIA implementation from independently reviewed
published facts, reviewed paraphrases to fresh workers (pages lead-only), and
isolated verified original-candidate Snow/VirtualBox trials needed for this
release. Excluded cached summary remains excluded; publication/reference/
physical hardware separate. Mac RAM/PC dirty presentation/controller synthetic
checks pass; native Mac/boot parity and final package remain pending.286 model
is committed7bb9856. Preserve24 originals/held brief/UART/old rows/assets/tags/
retired wiki and existing compatibility gates. Older held checkpoints below
are historical. [Evidence](docs/development/release2-integration-evidence.md).

2026-10-23,SRC-0399..0401/SPEC-0069/ADR-0080/IMPL-0071/TEST-0084:
bounded286 logical-bank/capability contract and host model;2 targeted CTests,
38 frozen checks/1596 flat cases/3 faulty controls pass. Production native paths
unchanged; no far backend/target run. Paired release remains primary; Mac binding
held for owner disposition after excluded cached summary constants, no bodies
or worker transfer. PC exact candidate verified, fresh GUI trial requested but
not run; CI pending. Preserve24 originals/held brief/UART/old rows/assets/tags/
retired wiki. [Checkpoint](docs/development/segmented-storage-evidence.md).

2026-10-23,SRC-0394..0398/ADR-0079/SPEC-0068/IMPL-0069/0070/TEST-0082/0083:
isolated supervisor exclusion callbacks (no ISR/MMIO/boot binding), plus C16
assert/chunk/flag/guest-offset facade.343 final local profiles; WCC delta9 calls/
6 helper-free objects/3 intentional refusals, no target run; Windows/68000 CI
passes at f69f27f, six native objects/matching twins, largest callback frame 4B.
Owner admits fresh worker after excluded compiler example; one fresh Claude
primary, lead direct small286 delta. General alignment/segmented views and all
M1-M7/compatibility gates remain. Preserve24 originals/held brief/UART/old rows/
assets/tags/retired wiki. [Checkpoint](docs/development/cpu-c16-evidence.md).

2026-10-23,SRC-0390..0393/ADR-0078:owner requires paired releasePRIMARY with
bounded286 progress alongside. SPEC-0066/IMPL-0068 foreground coordinator joins
inbox/capture through abstract short exclusion,unmasked owner calls;344 local
profile checks/12 audits/3 compiled defects pass; Windows/68000 CI passes at b079f20.
68000 compiler results: 90 objects/six closures, 40-byte descriptor and 72-byte
largest coordinator function frame. No target execution or full-stack fit claim. No CPU/
SCC/boot binding or target/release permission. SPEC-0067 C16 policy adopted,
8 TEST-0081 cases UNEXECUTED,prior compiler facts reused. One Claude primary,
one Grok distinct286 gap,no other fan-out. Preserve24 originals/held brief/UART/
old records/assets/tags/retired wiki and all gates. [Plan](docs/development/release-and-pc286-plan.md).

2026-10-23,SRC-0386..0389/ADR-0077/IMPL-0067/TEST-0079: isolated pinnedWCC16
compile preparation;39 final compiler calls/32 objects/7 expected rejections,
2 harness refusals,no link/target execution. C16 mask hazard confirmed atcompile
time,C11/helper/segmented-view/model/full-map/speed gaps remain;P286-1 partial.
One Claude DNS-context failure then network-authorized corrected primary;197
lines adopted with lead controls/harness. No runtime/core/CMake/boot input,
installer/firmware/emulator run or release. Preserve originals/held brief/UART/
old records/assets/tags/retired wiki and all gates. [Review](docs/development/pc286-toolchain-preparation.md).

2026-10-23, SRC-0383..0385 / ADR-0076: owner refines first286 goal to PC/AT5170
6MHz/640KiB/EGA and full hard-disk installation, no routine system floppy;
Word edition/fit/performance remain unqualified. Native performance is required
for Mac128K/68000, proposed286 and x64UEFI: shared C semantics, measured original
platform fast paths, no avoidable cost accepted merely for portability. Plan
only, no code/build/benchmark/target run/release; paired milestone and all gates
persist. [Plan](docs/development/native-performance-plan.md).

2026-10-23,SRC-0375..0382/ADR-0075:owner286 feasibility study complete,proposal only. Separate native16bit BIOS-first build plus original68k app runtime;true C16 toolchain/segmented guest views/int-width masks are prerequisites.640KiB planning profile/576KiB cap UNMEASURED;512x342 unscaled retained,CGA diagnostic candidate. One Claude primary,lead source/code checks;8 proposed checks UNEXECUTED. No install/code/build/target/release or current paired-milestone change. Preserve24 originals/held brief/UART/old records/assets/tags/retired wiki/all gates. [Study](docs/development/pc286-feasibility.md).

2026-10-23,SRC-0372..0374:SPEC-0065/ADR-0074 isolated pure inbox under IMPL-0066/TEST-0078. One Claude254-line primary,lead no-claim return fix and independent initial API tests;final test changes disclosed.343 final local profile checks/12 audits/three compiled defects pass;Windows/68000 CI passes atdcf4f42. No production coordinator,CPU/ISR/MMIO/boot binding,target trial/release/GIF. Original IC cases retain partial/unexecuted hardware scope,M1-M7/gates persist;preserve24 originals/held brief/UART/old records/assets/tags/retired wiki. [Evidence](docs/development/window-inbox-evidence.json).

2026-10-23,SRC-0369..0371:SPEC-0064/ADR-0073 Mac capture-to-foreground contract only. Separate16-record inbox/short claim before unmasked foreground owner;PRIMING closes baseline admission gap. Lead checks SCC controls and retains WR0 diagram/prose disagreement;no opcode/driver/CPU primitive admitted. One Grok primary,lead corrections,14 TEST-0077 cases UNEXECUTED. No code/header/fixture/build/trial/release/GIF;M1-M7 and existing gates persist. Preserve24 originals/held brief/UART/old records/assets/tags/retired wiki. [Review](docs/development/mac-scc-binding-review.md).

2026-10-22,SRC-0366..0368:SPEC-0063/ADR-0072 original ordered capture/drain helper implemented under IMPL-0065/TEST-0076. One Claude259-line primary,lead stop-path correction/static assertion and frozen API-only tests.343 final local CTests,12 audits/three compiled defects;first CI68000 passes but GCC Release test harness padding fails,reproduced/fixed using raw byte snapshots. No production change;Windows/68000 CI passes atd6e851f. No MMIO/ISR/ROM/EFI binding,target run,release or GIF. M1-M7 hardware/cost and all gates persist;preserve24 originals/held brief/UART/old rows/assets/tags/retired wiki. [Evidence](docs/development/window-capture-evidence.json).

2026-10-22,SRC-0362..0365:Mac motion prerequisite review admits narrow Zilog UM010902-0609 interface facts and rechecks SRC-0078/0080. SCC2.2us/access/latch limits,VIA read side effects/vector conflict explicit;M1-M7 remain partial/open. One Grok proposal adopted with no-coalescing/single-owner/lifecycle fixes;12 TEST-0075 cases UNEXECUTED. No API/code/fixture/build/trial/release. Next pure capture/drain contract/helper before MMIO. Preserve24 originals/held brief/UART/old rows/assets/tags/retired wiki and all gates. [Review](docs/development/mac-motion-prerequisites.md).

2026-10-22,SRC-0359..0361/TEST-0074:one exact CI-built SPEC-0062 PC candidate boots in isolated headless VirtualBox;16 decoded frames,4 accepted presses,three exact window translations,70s continuous interval/later input. Five reviewed original PNGs in Git. CWP1 read unavailable(public method not implemented);GUI/human/Mac parity and all gates persist. No runtime edit,new worker/release or further trial permission. VM closed,inventory/media restored;24 originals/held brief/UART/old records/assets/tags/retired wiki preserved. [Trial](docs/development/pc-window-boot-trial.md).

2026-10-22,SRC-0356..0358:SPEC-0062/ADR-0071 opt-in UEFI WindowDrawing binding/CWP1 trace under IMPL-0064/TEST-0073. One Claude missing-header refusal then corrected355-line artifact;lead integrates/fixes and verifies348 profile CTests/8 audits/4 compiled defects,unbooted EFI twins/media. Windows/68000 CI passed at0b23981. TEST-0072 keyboard codes/drain claims explicitly corrected. No target trial/release/BIOS;Mac M1-M7 and all gates persist. Preserve24 originals/held brief/UART bodies/old rows/assets/tags/retired wiki. [Evidence](docs/development/pc-window-boot-evidence.json).

2026-10-22,SRC-0353..0355:SPEC-0061/ADR-0070 firmware-neutral native PC window session implemented under IMPL-0063/TEST-0072. One Claude capped call yields adopted code/partial tests;lead fixes/finishes/integrates,329 profile CTests/10 audits/4 compiled defects pass. Windows/68000 CI passed atc863cf6. No boot binding,target trial or release;TEST-0070 full cases stay planned. Preserve originals/held brief/UART/old rows/assets/tags/retired wiki and all gates;Mac M1-M7 persist. [Evidence](docs/development/pc-window-session-evidence.json).

2026-10-22,SRC-0349..0352: SPEC-0060/ADR-0069 isolated native PC Mono1/one-row bridge implemented under IMPL-0062/TEST-0071. Grok primary times out without usable code;Codex direct fallback,Claude independent API-only oracle. No retry/fan-out;326 local profile checks/10 audits/3 compiled defects verified,Windows/68000 CI passed at4f44d3b. No window session,UEFI/ROM binding,target trial or release. Preserve originals/held brief/UART/old rows/assets/tags/retired wiki and all gates;Mac M1-M7 persist. [Evidence](docs/development/pc-mono-present-evidence.json).

2026-10-22,SRC-0346..0348: SPEC-0059/ADR-0068 PC native window boot-binding behavior finalized;TEST-0070 has32 unexecuted cases. One Grok primary,Codex sole integrator/verifier;RAM Mono canvas/one-row bridge,full UEFI gates,startup cancel before first publish,one receipt owner,bounded drain/terminal faults,dedicated trace. No code/header/fixture/build/target trial/release or BIOS ABI;Mac M1-M7 persist. Preserve originals/held brief/UART/old rows/assets/tags/retired wiki and gates. Next code proposal:isolated presentation bridge. [Review](docs/development/pc-window-boot-contract-review.md).

2026-10-21,SRC-0343..0345: reviewed current UEFI dependency inventory and proposed neutral payload boundary;one Grok primary,Codex sole integrator/verifier. Normalize memory,keep firmware-specific exit evidence and explicit mapping/native readiness;no ABI freeze,runtime/build change,BIOS code,trial or release. Paired window milestone remains immediate;Mac M1-M7 persist. Preserve all originals/held brief/UART/old records/assets/tags/retired wiki and gates. [Inventory](docs/development/unified-x64-boot-inventory.md).

2026-10-21,SRC-0341/0342,ADR-0067: owner adopts one native x86-64 PC build/distribution with identical shared OS payload and BIOS/UEFI entries;"any64-bit PC" is the coverage goal,not an achieved hardware claim. Documented roadmap only;paired-native-preview-2 remains immediate. No runtime input/ABI,BIOS build,trial,release or other gate change. Preserve originals/held brief/UART/old records/assets/tags/retired wiki. [Goal](docs/development/unified-x64-pc-plan.md).

2026-10-21,SRC-0338..0340: SPEC-0058 explicit per-poll PC packet/loss receipts under IMPL-0061/TEST-0069. Legacy API/layout preserved;fresh delivery independent of saturating counters. One Copilot test attempt times out with transport retries/no artifact;Codex original fallback,no retry/fan-out. Hosted/compiler/normal CI/Git/HQ only;changed PC controller requires fresh candidates,no window boot binding,target trial or release. Preserve originals/held brief/UART/old rows/releases/tags/retired wiki and all gates;Mac M1-M7 persist. [Receipt](docs/development/pc-bus-delivery-evidence.json).

2026-10-21,SRC-0336/0337: SPEC-0057 PC decoded-report handoff under IMPL-0060/TEST-0068. Validate/invertY/preservebuttons/delegateonce;direct small Codex task,no new worker. Hosted/compiler/normal CI only;no controller/boot/guest binding,target trial or release. Explicit packet freshness remains a prerequisite;Mac M1-M7 persist. Preserve originals/held brief/UART/old records/releases/tags/retired wiki and all gates. [Receipt](docs/development/pc-window-handoff-evidence.json).

2026-10-21,SRC-0332..0335: SPEC-0056 opt-in Windows hosted window preview under IMPL-0059/TEST-0067. Shared owner binding,unscaled512x342 inside640x480,capture/cancel and synthetic client-paint verification;owner additionally requests original PNG/GIF media in Git,one exact-identity GIF exception. Copilot primary,one later Grok compiler-gap review,Codex sole integrator/independent oracle/verifier. Audit-only compiler timestamp control retains full byte equality across delayed twins. No ROM/UEFI/guest ABI binding,target/VM/reference/hardware run or release. Preserve originals/held brief/UART/old rows/assets/tags/retired wiki and all gates. [Receipt](docs/development/hosted-window-preview-evidence.json).

2026-10-21,SRC-0329..0331: SPEC-0055 isolated shared native input owner under IMPL-0058/TEST-0066. One pointer event to counter/window/compositor;retained fault prefix,explicit cancel/full-repaint recovery. Copilot primary,Codex sole integrator/independent oracle/verifier. Hosted/compiler/normal CI only;no platform/boot/guest ABI binding,target trial or release. Preserve24 originals/held brief/UART/old rows/releases/tags/retired wiki and all gates. [Receipt](docs/development/window-input-evidence.json).

2026-10-20,SRC-0326..0328: SPEC-0054 isolated native window/cursor composition under IMPL-0057/TEST-0065. Hide/repaint/advance/show,shared nativeC,hosted/compiler/normal CI only. Copilot primary after Claude transport/cache-launch failures,Codex sole integrator/oracle/verifier. No input owner/platform/boot binding,target trial or release. Preserve24 originals/held brief/UART/old rows/releases/tags/frozen wiki and all existing gates. [Receipt](docs/development/window-composition-evidence.json).

2026-10-20,SRC-0323..0325: SPEC-0053 isolated original native window helper under IMPL-0056/TEST-0064:drag/content/cancel only,hosted/compiler/CI verification. One Claude attempt yields service retries/no artifact;Codex direct fallback and contract-first model. No scene/driver/guest ABI/ROM/EFI binding or target run/release. Preserve24 originals/held brief/UART/old records/assets/tags/frozen wiki and all existing gates. [Receipt](docs/development/window-interaction-evidence.json).

2026-10-20,SRC-0319..0322: implement SPEC-0051/0052 isolated native pointer/cursor helpers plus hosted/compiler/CI verification under IMPL-0055/TEST-0063. No scene/driver/binding/UART or target/reference/hardware/release change. Claude primary,Copilot cursor oracle(one empty attempt then reduced artifact),Codex direct small pointer test and sole integrator/verifier. [Actual evidence](docs/development/pointer-cursor-evidence.json). Preserve24 originals/held brief/old records/releases/wiki;existing Mac M1-M7,PC GUI/human and compatibility gates persist.

2026-10-19 cursor contract,SRC-0316..0318: SPEC-0052 / ADR-0060 and TEST-0062 unexecuted cases reviewed for paired preview2;no code/art/fixture/test execution/build/trial/release. One Claude primary plus one focused correction,no second worker. [Contract](docs/specifications/SPEC-0052-native-cursor-composition.md). Existing source/compatibility/PC GUI/Mac M1-M7 gates remain;current releases and originals preserved.

Owner priority: protect Codex usage. Count coordination, reading, review and bookkeeping as costs. Do small tasks directly when delegation adds comparable overhead; delegate substantial bounded work and integrate sound artifacts. Keep required records concise, reuse unchanged context/evidence, and preserve clean-room and verification gates. External worker activity does not prove net savings. See [orchestration](docs/development/agent-orchestration.md).

2026-10-19 source-review continuation, SRC-0314/0315: owner clears narrow published mouse review; earlier hold resolved, declaration excluded. First pointer/input contract/inventory slice complete with Mac adapter gaps M1-M7; no code, tests, build or trial. Next recommended: bounded cursor composition contract. [Review](docs/development/mac-pointer-source-review.md). Older held checkpoints below are historical. Existing gates and released capabilities remain unchanged.

Current pointer/input contract2026-10-19,SRC-0310..0313: SPEC-0051 original native pointer/input contract and68 unexecuted TEST-0061 case identifiers reviewed for paired-native-preview-2. Shared signed-safe coordinates,one-owner edge/cancel/suppression,counter mirror and future bounded cursor composition requirements;no code/header/fixtures/test execution/build/binding/target run/release. PC evidence inventory distinguishes hosted,E2device,E3GUI,E4human;no input fix claimed. Mac motion source review held for owner disposition after one unrelated published-manual constant-declaration peek;no function bodies,no adopted declaration,no worker/manual exposure or affected implementation. Current releases/24 originals/held brief/UART/wiki preserved;OneNote/wiki retired and existing compatibility gates persist. [Review](docs/development/pointer-input-contract-review.md). Mac source lane awaits the owner decision;independent contract/evidence work is complete,not a hardware implementation clearance.

Current next-release planning2026-10-19,SRC-0307..0309: Plan only for paired-native-preview-2: recommended original moving cursor plus one draggable/content-click native window on shared C compiled68000/x86-64,logical512x342;provisional ROM0.4.0-alpha.1/PC0.2.0-alpha.1. First proposed slice is pointer/input contract and Mac motion-source sufficiency/PC GUI evidence inventory. No implementation,fixtures,tests,build,target run,version-pointer change or release;all acceptance checks unexecuted. Current published pair/assets/tags,24 originals/held brief/UART/wiki and all compatibility gates preserved;OneNote/wiki retired. [Proposed goal](releases/paired-native-preview-2.md).

Completed paired prerelease publication2026-10-19,SRC-0305/0306: Published paired ROM0.3.0-alpha.1 and native x86-64 PC0.1.0-alpha.1 experimental prereleases from approved metadata d9c2341. Both annotated tags select that commit; compiled source c631b824 and qualification87e95de stay distinct. Eight draft and eight public downloads match exact approved sizes/SHA-256 and GitHub digests;previous releases unchanged,neither stable latest. No image edit,rebuild,target run or new worker. Owner accepts disclosed preview limits for publication;PC human capture/GUI,Snow preferences,physical/full128K/historical/edition/B2/native-app/Linux/both System1-ROM gates remain open. System unreleased;24 originals/held brief/UART/wiki preserved,OneNote/wiki retired. [Receipt](releases/paired-native-preview-1-publication.json). No future publication or trial follows from this completed scope.

Final paired package review2026-10-19,SRC-0301..0304: Final package review only: unchanged exact CI/boot-observed native68000 ROM and native x86-64 EFI/raw disk; corrected component-specific offline notes, complete c631b824 source and GPL text, deterministic demo ZIPs/checksum inventories, independent audit and37 actual checks. Both proposed prereleases remain unpublished pending explicit owner approval and acceptance of PC human capture/GUI and other stated preview limits. No runtime edit, rebuild, target run, tag or release. [Approval proposal](releases/paired-native-preview-1-approval.md).

Current owner preview-session repair2026-10-19,SRC-0298..0300: "Do it" accepts bounded SPEC-0050 continuous explicit PC outline preview,real-loop tests/compiler/CI and automated own-build verification/Git/HQ. Mac already continuous,unchanged. Old timed/Escape modes and all faults preserved; no UART/decoder/controller/shared scene changes. Claude primary,Codex sole integrator/verifier; no redundant worker. Prior PC human misses remain unresolved,not a synthetic pass. Preserve24 originals,held brief,prior evidence/releases/tags/wiki; OneNote/wiki retired. No reference/hardware/release expansion; existing source/role/historical/full128K/B2/native-app/Linux/both System1 gates persist. [Evidence](docs/development/preview-session-evidence.md).

Current paired preview trials2026-10-19,SRC-0295..0297: exact SPEC-0049 candidates booted once in Snow/twice in isolated VirtualBox; all closed. Mac human2 clicks pass. PC human clicks unreliable/fail; synthetic21/21 duration matrix plus Space passes,not human qualification. PC stops input at diagnostic second60; lifecycle/capture cause unresolved. Owner clears automated tests without human participation; scope current original-image verification,no implementation/reference/release expansion. Codex sole writer/verifier,no new workers needed. Preserve24 originals,UART,old rows/releases/tags/wiki; OneNote/wiki retired and existing gates persist. [Trial](docs/development/paired-preview-boot-trial.md).

Current owner release cadence2026-10-19,SRC-0291: continue current work while always advancing a named paired nativeMac68000/x86 PC release candidate. paired-native-preview-1 targets draft ROM0.3.0-alpha.1/PC0.1.0-alpha.1 with matching capability/readiness checks; current versions/published assets unchanged. This slice accepts SPEC-0049 explicit paired native outline workload/bindings,independent tests/compiler/candidates,local DRAFT packaging and ordinary Git/HQ. Codex sole writer/integrator/verifier,Claude primary,Copilot independent oracle. No target/VM/emulator/reference/hardware run,tag or publication. Preserve24 originals/copies,UART,prior modes/records/releases/tags/wiki; OneNote/wiki retired,no chats/subagents/automation. Existing source/role/historical/edition/physical/full128K/B2/human/native-app/Linux/both System1 gates persist. [Target](releases/paired-native-preview-1.md).

Current owner outline implementation2026-10-18/19,SRC-0287..0290: "OK, move on to the next task" and "continue" accept the proposed SPEC-0048 separate native helper,independent tests/compiler/CI and ordinary Git/HQ. Claude primary,Copilot independent frozen-API oracle,Codex sole writer/integrator/verifier. Existing headers/core/workloads/ROM/EFI inputs/candidates unchanged; no guest binding/full manager,target/reference/hardware run or release. Preserve24 originals/copies,UART,old rows/releases/tags and frozen wiki. No OneNote/wiki,chats/subagents/automation;source/edition/physical/full128K/B2/human/native-app/both System1 gates persist. [Evidence](docs/development/native-outline-evidence.md).

Current owner screenshot publication and next outline contract (2026-10-18,SRC-0280..0286):
Store reviewed own-product evidence PNGs in docs/evidence/screenshots with indexed
manifest/hashes/captions/rights/source pins and research/test links for the morning
updater.7 prior SPEC-0047 captures are publication copies,no new target run.
Implement narrow screenshot guard/tests/CI only; SPEC-0048 / ADR-0054 is contract
only,44 unexecuted outline cases,H1-H5 open,no runtime/header/build/trap/scene
change. Argon4 High timed out,no artifact; one Claude guard fallback and independent
Copilot tests,Claude outline contract. Codex sole writer/integrator/verifier.
Preserve24 originals/copies,UART,old rows/releases/tags/wiki and existing gates.
No OneNote/wiki,chats/subagents/automation,new trial or release. [Evidence](docs/development/screenshot-outline-evidence.md).

Current owner paired selected-port boot observations (2026-10-18,SRC-0278/0279):
"Continue" accepts the proposed one Snow Early128K and one isolated native PC
trial of exact SPEC-0047 candidates,synthetic input/captures and ordinary Git/HQ.
Both sessions are now closed; no further trial,build,release or runtime scope is
authorized. Native drawing/counters observed; Snow persistent preference isolation
and human/physical input remain unqualified; Mac empty/PC timed progress differ.
24 originals/copies,UART,old records/releases/tags/wiki preserved; all existing
source/edition/physical/full128K/B2/native-app/both System1 gates persist. No
OneNote/wiki,extra chats/subagents/automation. [Trial](docs/development/paired-selection-boot-trial.md).

Current owner paired selected-port demonstration (2026-10-18,SRC-0274..0277):
"Do it" accepts SPEC-0047 original native68000/x86 two-port visual workload,
explicit Mac/PC candidate modes,independent tests/compiler/CI and ordinary Git/HQ.
Shared branding,logical512x342/PCcentering and counter behavior persist; old modes
and release assets retain behavior. Claude primary C/bindings,Copilot independent
pixel/scene oracle,Codex sole writer/integrator/verifier. No target/emulator/VM/
reference/hardware trial or release; source/edition/physical/full128K/B2/human/
both System1 gates persist. Preserve24 originals/copies,UART,old rows/releases/
tags/frozen wiki. No OneNote/wiki,extra chats/subagents/automation.
[Evidence](docs/development/paired-selection-evidence.md).

Current owner selected-port drawing scope (2026-10-18,SRC-0270..0273):
"Continue with the next task" accepts SPEC-0046 opt-in drawing through current
prepared port,original native68000/x86 code,independent tests/compiler/CI and
ordinary Git/HQ records. Claude primary delta; Copilot independent frozen-API
pixel/routing oracle; Codex sole writer/integrator/final verifier. Preserve old
default/fixed/selection modes and UI workloads. No full manager/global registry/
JSR/callback/SDK expansion,target/reference/hardware run or release. Preserve24
originals/copies,UART,old rows/releases/tags and frozen wiki; existing gates persist.
OneNote/wiki retired; no extra agents/chats/automation.
[Evidence](docs/development/selected-drawing-evidence.md).

Current owner integrated port-selection scope (2026-10-17,SRC-0266..0269):
"Next step" continues SPEC-0042/0043/0044 through SPEC-0045's actual native owner,
canonical GetPort/SetPort coordinator, original selection-only legacy fixture and
independent tests/compiler/CI/ordinary Git/HQ. Claude primary API/code; Copilot
frozen-API independent oracle; Codex sole writer/integrator/final verifier. Old
default/fixed-port semantics and UI workloads persist; new opt-in mode only.
No selected-port drawing/full manager/global registry/JSR/callback/SDK expansion,
target/reference/hardware trial or release. Preserve24 originals/copies,UART,
old rows/releases/tags and frozen wiki; current compiler inputs may change and
old boot observations do not transfer. No OneNote/wiki,agents/chats/automation.
[Evidence](docs/development/port-owner-evidence.md).

Current owner prepared-port ownership contract (2026-10-17,SRC-0262..0265):
"Next task" accepts SPEC-0044 / ADR-0050 P1/P2/P4 owner integrity/environment-span/
lifetime/admission/retirement contracts and unexecuted cases only. No code,fixture,
runtime test,binding,build,target trial or release. Claude primary draft; one Grok
stopped-lifetime/handback review; Codex sole writer/integrator/verifier. Prospective
profile only; old runtime contracts,24 originals/copies,64 native inputs,UART,
prior rows/releases/tags/wiki preserved. No OneNote/wiki,agents/chats/automation.
[Review](docs/development/prepared-port-owner-lead-review.md).

Current owner native selection implementation (2026-10-17, SRC-0258..0261):
"get started" accepts SPEC-0043 pure shared native68000/x86 selection values,
original code, independent API-only tests, paired compiler/CI verification and
ordinary Git/HQ publication. Claude primary core; Copilot independent tests;
Codex sole writer/integrator/final verifier. Freeze API before workers. No guest
trap binding, owner lifecycle/header/environment-span amendments, workload/ROM/EFI
input changes, target trial or release. Preserve24 originals/copies, UART bodies,
old rows/releases/tags and frozen wiki. No OneNote/wiki,agents/chats/automation.

Owner capability-audit scope (2026-10-17, SRC-0252..0257): all four external CLIs
reported Windows11 capabilities and reviewed one identical proposal. Final routing
plan is [here](docs/development/tool-audit-2026-10-17/final-plan.md); normal tasks
retain one primary/optional distinct-risk worker. Argon always High, Codex sole
writer/integrator/final verifier. No feature/source/input/VM/hardware/release gate
expansion, runtime code/build/fixtures, agents/chats/automation or OneNote/wiki work.
Preserve originals/copies, native inputs, UART, releases/tags and old records.

Current owner guest drawing-port contract scope (2026-10-17, SRC-0244..0248):
"Next step" accepts the planned review/contract slice after SPEC-0041. SPEC-0042
defines prepared GetPort/SetPort selection, one native68000/x86 semantic core,
strict guest ownership and explicit future integrity/environment-span prerequisites.
No implementation, fixtures, tests, registry entries, builds, trial or release in
this slice. Claude primary contract; Grok distinct ownership review; Codex sole
integrator. Preserve24 originals/copies, all runtime/build candidates, UART bodies,
older records/releases/tags and frozen wiki. OneNote/wiki permanently retired;
all source/role/edition/physical/full128K/B2/human/System1 gates persist. No agents.
[Lead review](docs/development/guest-port-lead-review.md).

Current owner paired drawing-context scope (2026-10-17, SRC-0240..0243): "Next step"
continues the planned native drawing-port/clipping foundation: SPEC-0041 original
native borrowed view, coordinate offset and rectangular clip; same C on68000/x86,
real native workload use with unchanged scene. Claude primary core/binding draft,
Copilot independent geometry tests; Codex sole integrator. Original code/tests,
compiler/CI/ROM/EFI candidates and ordinary Git/HQ records. No target trial/release,
guest GrafPort/GetPort/SetPort ABI, regions, cursor/events or SDK/loader expansion.
Preserve24 originals/copies, UART bodies, old helper/legacy fixture/released assets,
prior records/tags and frozen wiki. New native candidate inputs may change; old
boot observations don't transfer. OneNote/wiki remain retired; source/role/edition/
physical/full128K/B2/human and both System1 boot gates persist. No Codex subagents.


Completed owner PC mouse trial (2026-10-17, SRC-0238/0239): "Continue" accepted
the proposed single isolated native PC boot/input trial. That exact SPEC-0040
candidate reached READY, passed direct keyboard/mouse API edge/held/release checks,
retained static scene pixels and ended normally at guest second60/count3. Session
closed. Desktop automation delivery and human input remain unqualified; pauses
prevent uninterrupted timing claims. No new code/Mac trial/release. Future target
runs need fresh owner go-ahead. Prior compiled records retain their dated scope;
all source/UART/edition/physical/full128K/B2/historical/System1 boot gates persist.
OneNote/wiki remain retired. [Observation](docs/development/pc-mouse-boot-trial.md).

Current owner controller implementation (2026-10-16, SRC-0233..0237): "Do it" accepts SPEC-0040
single-owner8042 mouse startup/routing, original code/hosted/compiler/CI/EFI
candidate checks. Explicit PC mouse build profile preserves default legacy
keyboard APIs/trace and all UART implementation/test bodies; no new target trial,
release, native ABI/SDK or cursor/Event Manager expansion. Claude primary code,
Copilot independent sequencing tests; Codex sole writer. Existing service/input,
source/role/edition/physical/full128K/boot gates persist. Preserve24 originals/
copies, Mac ROM inputs, old rows/releases/tags and frozen wiki. OneNote/wiki
remain permanently retired. No extra Codex chats/subagents/automation.


Current owner continuation (2026-10-16, SRC-0227..0232): proceed with SPEC-0039 shared native
mouse-button core, pure PS/2 report decoder and Windows/native-Mac bindings,
original hosted/compiler/CI/candidate checks; freestanding PC controller startup
and routing follow separately. Claude primary/Copilot independent tests under
standing reviewed-public/original-only service permission; Codex sole writer.
No new emulator/VM/hardware/reference run or release. Preserve originals/copies,
UART bodies and older releases/records. No extra Codex chats/subagents/automation.
The owner permanently retired OneNote AND its Git wiki queue for this project:
do not contact it or maintain/add wiki/queue/manifest records. Existing wiki files
are frozen historical records. This supersedes every older wiki/"update wiki"
instruction below; ordinary project/HQ development/provenance records continue.


Current owner orchestration extension (2026-10-16, SRC-0223..0226):
highest priority conserve ChatGPT usage. Antigravity/Copilot join Claude/Grok
under reviewed-public/original-only inputs. One substantial primary artifact,
second worker only distinct risk; Codex sole writer, decisions/integration/final
checks. Reuse existing qualifications. Current continuation is SPEC38 verification
maintenance only, no runtime/driver/ABI/trial/hardware/release scope expansion.
Preserve originals/copies, UART bodies and prior records/releases. No new Codex
chats/subagents; ordinary Git/HQ/wiki queue. OneNote and its Git wiki queue are retired; see current owner scope above.
[Protocol](docs/development/agent-orchestration.md); [qualification](docs/development/worker-orchestration-qualification.md).

Current owner native workload implementation (2026-10-16, SRC-0219..0222):
"move on to the next task" with best use of Claude/Grok authorizes SPEC-0038 /
ADR-0044 / IMPL-0042 / TEST-0046 direct native drawing on Mac/PC, standalone
closure, original tests/compiler/CI/candidates and ordinary Git/HQ/wiki records.
Codex sole writer, one bounded Claude test artifact and one Grok boundary report
under standing reviewed-public/original transfer permission. Separate40-byte
legacy fixture/--drawing profile remain. Native --native-drawing ROM and PC
images are candidates, no new trial/release or external native ABI/SDK/loader.
Preserve24 originals/copies, UART bodies, prior assets/tags/records. Fresh owner
go-ahead before emulator/VM/reference runs; PC mouse/event parity remains open.
No extra agents/chats/automation/OneNote. Earlier policy-only paragraph is dated.
[Evidence](docs/development/native-drawing-evidence.md).

Current owner native-parity correction (2026-10-16, SRC-0217/0218): complete
the68k experience on Mac and advance the native x86 PC port with every matching
capability. On PC the OS/managers are native x86; the interpreter serves legacy
68k applications. This supersedes earlier blanket "68k complete first" native
application/test/loader prohibitions wherever they block native PC counterparts
or original parity workloads. No whole-Mac-completion prerequisite for that work.
Detailed native ABI/SDK/loading still needs reviewed feature contracts; no broad
third-party execution admission or current implementation is implied. Current
task records policy/gaps only, no code/build/new trial. Preserve24 originals/
copies, UART bodies, old releases/tags/records. Fresh go-ahead for each future
emulator/VM/reference trial; no extra agents/chats/automation/OneNote.
[Authoritative paired plan](docs/development/native-parity-plan.md). Earlier
dated gate/result paragraphs are historical where superseded by this correction.

Completed ROM0.2.0-alpha.1 publication (2026-10-16, SRC-0215/0216): four
draft/public downloads and GitHub digests match; annotated tag verified. Uses
exact previously observed shared-scene/drawing ROM and matching original
source. First alpha immutable; System unreleased. Publication authorized by
the owner's next-release request. No new build/trial or gate waiver.
[Receipt](releases/rom/0.2.0-alpha.1-publication.json).

Current owner ROM release scope (2026-10-16, SRC-0215/0216): "OK, I think
this warrants another rom release." Publish ROM0.2.0-alpha.1 as an experimental
prerelease, using only the already boot-observed shared-scene ROM, matching
original source and reviewed setup/notes/records/checksums. Annotated rom-v0.2.0-alpha.1
tag and GitHub release permitted. System remains unreleased. No new build,
emulator/VM/reference trial, native app, workers/agents/chats/automation/OneNote.
Preserve24 originals/copies, UART bodies, prior assets/tags and all existing
source/historical/physical/full128K/parity/edition/B2/human/boot gates.
[Release notes](releases/rom/0.2.0-alpha.1.md).

Completed screenshot sessions (2026-10-16, SRC-0213/0214): owner requested
"a screenshot for each of these for use in a blog post." One isolated own-ROM
Snow128K boot and one own-image VirtualBox boot show the current branded scene,
APP/097, drawing and six phases. PC512x342 is centered in1024x768. Both sessions
are closed; no input test or future run authorized. Earlier compiled-only records
retain dated meanings. [Observation](docs/development/shared-product-scene-boot-evidence.md).

Current owner display scope (2026-10-16, SRC-0209/0210/0211): implement one
portable monochrome scene and supplied-brand approximation for drawing-ROM,
native-PC and hosted Windows targets. All PC work targets a logical512x342
image, centered in the reported framebuffer, until the owner says otherwise.
640x480 is a checked case, not a mandatory hardware mode. SPEC-0037 / ADR-0042 /
IMPL-0041 / TEST-0045; ordinary original code/tests/compiler/CI/builds and Git/HQ/
wiki records. Existing own bootstrap/native code and68k fixture are permitted;
native x86 application/SDK gate stays closed. Preserve24 originals/copies, UART
bodies, old records and released alpha assets/tag. No new emulator/VM/reference
trial without fresh explicit go-ahead; no agents/chats/automation/OneNote.
[Standing display requirement](docs/development/display-target.md).
Earlier drawing trials are closed and retain their exact earlier images.

Completed drawing trial (2026-10-16, SRC-0207/0208): owner said
"Yes—run the isolated drawing-ROM trial". One isolated own-ROM Snow128K cold
boot displays APP/097, hollow rectangle and six phases; counter0000/0001 and a
later0001 capture retain exact result-panel pixels. Trial now closed. Automated
input delivery remains unqualified; owner unavailable, no human input credited.
Future emulator/VM/reference trials need fresh explicit go-ahead. No new release,
historical/physical/full128K or System1/ROM gate closes. [Observation](docs/development/drawing-app-boot-trial.md).
Earlier implementation and compiled-only checkpoints retain their dated scope.

Current owner drawing continuation (2026-10-16, SRC-0202): "Move on to the next
thing." continues to the next visible milestone in docs/milestones.md: an
original loaded68k application drawing through selected APIs. SPEC-0036 /
ADR-0041 permits the fixed128x64 mono PaintRect/EraseRect profile, original
program/tests, compiler/CI and own-ROM candidate construction with ordinary
Git/HQ/wiki records. Codex sole writer; one bounded Claude helper artifact and
one Grok boundary check under standing SRC-0149. No global QuickDraw manager,
guest GrafPort/InitGraf/regions, SDK/native application, new release or trial.
Any emulator/VM/reference trial still needs fresh explicit owner go-ahead.
Preserve24 originals/copies, released alpha assets/tag and old records; source/
UART/historical/physical/full128K/parity/edition/B2/human and both boot gates
remain. No extra agents/chats/automation/OneNote. Earlier scopes stay dated.

Current owner ROM alpha publication (2026-10-16, SRC-0200): "OK, move on."
accepts the proposed experimental ROM0.1.0-alpha.1 publication after final
source packaging, Snow instructions and package/provenance review. Publish only
the already observed own-ROM bytes and reviewed original source/text assets,
using an annotated rom-v0.1.0-alpha.1 tag and GitHub prerelease. Keep System
unreleased, historical/human provenance limitations explicit and all24 originals/
copies, native inputs and previous records intact. No new binary build, trial,
native application, workers/agents/chats/automation/OneNote. Existing source/
UART/physical/full128K/parity/edition/B2/human and both System1/ROM gates persist.
This supersedes the groundwork-only publication limit for this one ROM alpha.

Current owner release groundwork (2026-10-15, SRC-0198): "I think the rom
should be its own release..." and "ok, do the groundwork for this now."
authorize separate ROM/System versions, histories, milestone/compatibility
records and a local draft preparation tool, with ordinary public/HQ/wiki records.
ROM 0.1.0-alpha.1 is a draft of the already observed image; System stays
unversioned/unreleased. No tag, GitHub release, download publication, new binary,
boot/reference trial, native application or publishing automation. Preserve all24
research originals/copies, qualified inputs/artifacts, prior records and source/
UART/physical/full128K/parity/edition/B2/human gates and both System1/ROM goals.
No new agents/chats/OneNote. [Release policy](releases/README.md).
Earlier scopes retain their dated meaning.

Completed redraw trial (2026-10-15, SRC-0196/0197): explicit owner approval
"Yes—test the redraw ROM in Snow" covers one now-closed isolated own-ROM run.
The complete result and counter updates are observed without full-scene redraw
at captured points; some automated input delivery remains unresolved. No all-click,
historical, physical or human-input qualification. Future trials need fresh go-ahead.

Current owner redraw scope (2026-10-15, SRC-0193): "OK, next step." accepts the
proposed focused rendering-speed fix within SPEC-0035 / IMPL-0039 / TEST-0043,
with original regression/compiler/CI checks and ordinary public/HQ/wiki records.
Codex implements and remains sole writer; one bounded Claude test artifact under
SRC-0149, further worker calls only for a concrete gap. Optimize the native
overlay and counter update; no CPU/loader/guest ABI/QuickDraw/native-app expansion.
Future Snow/VM/reference trial still needs fresh explicit go-ahead. Preserve24
originals and existing byte-identical private copies, old evidence/edition rows
and source/UART/physical/128K/parity/B2/human gates and both System1/ROM targets.
No new chats/agents/automation/OneNote. Earlier scopes retain dated meaning.

Completed isolated trial (2026-10-15, SRC-0191/0192): owner explicitly said
"Yes—run the isolated Snow boot trial". One original-ROM Macintosh128K cold
boot passes APP/097/pattern/six phases and one automated kernel mouse event.
The trial is closed. This grants no additional emulator/VM/reference run or
historical/System1/physical claim. Future trials still need fresh go-ahead.

Current owner integration scope (2026-10-15, SRC-0187): "OK, let's move on then."
accepts the proposed visible milestone: one independently authored68k resource
application loaded/executed through our machinery, with its observed result in
our boot ROM. Implement SPEC-0035 as one coherent integration, focused tests,
compiler/regression/CI and one public/HQ/wiki checkpoint; batch the former X-2
through X-5 demo prerequisites rather than separate contract-only cycles. Codex
sole writer; Claude primary original demo artifact and Grok one boundary check
under SRC-0149. Own loaded-app ROM construction/integration is authorized.
No QuickDraw/desktop/native application or historical compatibility claim follows.
Emulator/VM/reference runs still require an explicit go-ahead before that trial.
Preserve24 originals/copies and source/UART/physical/full128K/parity/edition/B2/
human gates and both System1/ROM targets. No extra agents/chats/automation/OneNote.
Older scopes retain their dated meaning.

Current owner implementation scope (2026-10-15, SRC-0183): "Do it."
accepts SPEC-0034 dependency X-1: original guarded interpreter access/class/
proposed-state checks before staged commit, focused tests, one Claude test
artifact and one Grok bypass check, compiler/regression/CI and ordinary public/
HQ/wiki records. Codex remains sole repository writer. This supersedes the
prior contract-only limit solely for X-1. No X-2..X-5 bridge, application/image/
session entry, reset/boot binding, native app or VM/emulator/reference trial.
SRC-0149 scoped inputs persist; preserve24 originals/copies and all older gates.
No extra agents/chats/automation/OneNote. [Evidence](docs/development/guarded-interpreter-evidence.md).
Earlier entries retain their dated scope.

- Current owner contract scope (2026-10-15, SRC-0179): "OK. Carry out your plan
  for the next task. GO." accepts the controlled owned-image execution contract
  and efficient repo-based assignments: Claude drafts, Grok checks one risk,
  Codex retains one repository writer, decisions, verification and publication.
  [Task brief](docs/development/tasks/owned-image-execution.md). Initial budget
  two supervised CLI calls with bounded selective reads; current scoped public/
  original input permission SRC-0149 persists. No implementation/fixtures/new
  TEST/IMPL/build, guest/application execution, reset/boot integration, native
  app/SDK, emulator/VM/reference trial or prohibited inputs. Preserve24 originals/
  copies, compiled/build/edition/UART/source/native/physical/full128K/parity/B2/
  human gates and both untested ROM/System1 targets. No extra agents/chats/
  scheduled automation/OneNote. Earlier scopes retain their dated meaning.

- Current owner implementation scope (2026-10-15, SRC-0175): "Go for it"
  accepts the original SPEC-0033 loading/ownership/cleanup implementation and
  tests, bounded Claude/Grok reviews, compiler/regression/CI verification and
  ordinary public/HQ/wiki records. This supersedes the previous contract-only
  limit for this slice. Application execution, guest bindings, reset/boot
  integration, native app/SDK and VM/emulator/reference trials remain outside
  scope. Preserve all24 originals/copies and prior source/UART/native/physical/
  full128K/edition/B2/human gates. No extra agents/chats/automation/OneNote;
  SRC-0149 scoped reviewed-public/original-code CLI transfers apply.

- Current owner contract scope (2026-10-15, SRC-0171): "OK, do it" accepts the
  proposed safe segment loading/ownership/cleanup contract after verified
  preflight. SPEC-0033 / ADR-0037 defines eager image staging/population/retirement
  and keeps execution admission deferred. No implementation, fixture, new
  TEST/IMPL/build, guest ABI/binding, app execution, boot change, native app/SDK
  or VM/emulator/reference run. Scoped read-only Claude/Grok review and ordinary
  Git/HQ/wiki records; preserve all24 originals/copies, compiled pointers and
  existing source/UART/native/physical/full128K/edition/B2/human gates. No extra
  agents/chats/automation/OneNote. Earlier scopes retain their dated meaning.

- Current owner implementation scope (2026-10-15, SRC-0167): "OK, move on to
  the next task" continues SPEC-0032 into its original read-only hosted CODE
  inspection/layout checker, original conformance/compiler/regression/CI checks,
  bounded Claude/Grok review and ordinary public/HQ/wiki records. This supersedes
  the prior contract-only limit solely for this checker. No loader, heap reset,
  guest binding/execution, native application/SDK, boot change or VM/emulator/
  reference run. Preserve all24 originals/copies and prior source/UART/edition/
  physical/full128K/B2/human gates. SRC-0149 transfers apply; no extra agents,
  chats, automation or OneNote. Earlier entries retain their dated scope.

- Current next contract (2026-10-14, SRC-0163/0165/0166): after completing
  resource residency and separately recording the owner's two boot requirements,
  SPEC-0032 / ADR-0036 finalizes read-only CODE inspection/layout preflight only.
  [Review](docs/development/code-preflight-contract-review.md). No implementation,
  fixture, TEST/IMPL/build, guest ABI/registry/loader/native application or new
  VM/emulator/reference run. Preserve all24 originals/copies and prior compiled/
  edition/native/source/UART/physical/128K/B2/human gates. Scoped Grok review and
  ordinary Git/HQ/wiki records; no extra agent/chat/automation/OneNote. The
  completed resource implementation permissions remain dated history.

- Current owner continuation (2026-10-14, SRC-0163/0164): finish the verified
  resource checkpoint, then the bounded CODE-only launch/preflight contract;
  record original System1 disks on our ROM and our System1 on an identified
  supported original Apple ROM as two mandatory unverified boot targets.
  [Requirements](docs/development/system1-rom-boot-requirements.md). This adds
  future black-box interoperability, not Apple implementation inputs, dependency
  or redistribution. Source/platform admission and a fresh owner go-ahead still
  precede any actual emulator/VM/reference trial. No implementation/fixture/
  guest ABI/native application/SDK follows from this contract scope. Preserve
  all24 originals/copies and existing source/UART/physical/edition/B2/human gates;
  scoped Claude/Grok reviews and Git/HQ/wiki records only, no extra chats/agents/
  automation/OneNote. Earlier completed implementation scopes remain dated.

- Current owner scope (2026-10-14, SRC-0159): "ok do it" accepts the proposed
  original SPEC-0031 resource-loading implementation, ownership/cleanup tests,
  bounded Claude failure/Grok coverage reviews, compiler/regression/CI checks
  and ordinary public/HQ/wiki records. SRC-0149 scoped transfers apply. No guest
  binding, CODE loader/execution, native application, new VM/emulator/reference
  trial, UART body work, prohibited input, extra agent/chat/automation/OneNote.
  Preserve all24 research originals/copies and prior build/edition evidence.
  [Implementation/evidence](docs/development/resource-residency-evidence.md).
  Earlier contract-only and implementation entries retain their dated scope.

- Current owner scope (2026-10-14, SRC-0155): "Do it" accepts the proposed
  Resource Manager contract before CODE loading. SPEC-0031 / ADR-0035 defines
  original hosted residency/cache/ticket/source/fault/lifecycle ownership only.
  Bounded read-only Claude/Grok reviews under SRC-0149 and ordinary Git/HQ/wiki
  records are allowed. No implementation/fixtures/runtime tests/TEST/IMPL/build,
  guest ABI/registry/loader/boot binding, native application, VM/emulator/reference
  run, UART body work, prohibited input, extra agent/chat/automation/OneNote.
  Preserve all24 research originals/copies; prior runtime/build/edition evidence
  stays exact. Human provenance review and historical/full128K/physical/B2 gates
  remain open. [Lead review](docs/development/resource-residency-contract-review.md).
  Earlier implementation permissions are dated completed slices.

- Current owner scope (2026-10-14, SRC-0151): "Do it" approves the proposed
  original bounded SPEC-0030 hosted heap implementation, supervised Claude
  failure/Grok coverage reviews, original hosted/compiler/regression/CI checks
  and ordinary public/HQ/wiki evidence. SRC-0149's scoped transfers apply.
  No loader, new guest binding, native application, VM/emulator/reference run,
  UART body work, prohibited input, extra agent/chat, automation or OneNote.
  Preserve all24 untracked research originals/copies. Human review and
  historical/physical/full128K/edition/B2 gates remain separate.
  [Implementation/evidence](docs/development/heap-lifecycle-evidence.md).

- On 2026-10-14 the owner requested Codex orchestration of Claude and Grok and
  explicitly approved "Yes—this review and future scoped tasks" (SRC-0149/0150).
  Standing permission covers relevant reviewed public ClasSICK-0.97 contracts
  and original project code sent to Anthropic/Claude or xAI/Grok within the
  active owner-authorized slice. Exclude HQ, untracked originals, manuals,
  credentials and prohibited inputs. No extra subagents/new Codex chats/
  automation/OneNote/native-app/VM or task expansion. This supersedes the older
  sole-Grok/no-delegation limit only for these supervised CLI workers. Codex
  retains decisions, one writer, verification and publication. The qualification
  read one public spec and yields SPEC-0030 v2's wording/state clarification,
  still unimplemented. All22 originals/copies and compiled/interface/boot inputs
  are preserved; human provenance review and historical/edition gates persist.

- On 2026-10-14 the owner requested Grok capability/usage assessment, separate
  existing Snow/VirtualBox boot screenshots, then "Thank you. Now, move on to
  the next tasks" and explicitly "Yes—approve this Grok review" for the named
  public document excerpts sent to xAI (SRC-0146..0148). SPEC-0030 / ADR-0034
  finalizes the hosted heap lifecycle contract only; no code/fixture/TEST/IMPL/
  build or loader/reset binding. Preserve all22 unchanged/untracked originals
  and private copies. Bounded Grok document review is the sole new delegation
  exception; no further task/agent/chat/automation/OneNote or VM/reference run
  follows. Existing source/UART/native/physical/128K/edition/B2 and human-review
  gates persist. Earlier dated permissions and intake limits retain their scope.

- On 2026-10-14 the owner requested the research-only re-entry/event/package
  intake (SRC-0142..0145). All17 originals remain unchanged/untracked; keep
  verified private copies/manifest and the completed dispatcher separate. No
  reset/stash/clean/pull, new spec/ADR/test/implementation/fixture/scenario/code/
  interface import or new implementation permission. The lead review records
  RA/ES/PG choices and34 open historical labels, with sample material excluded.
  Only ordinary Git/HQ/wiki intake; existing source/UART/native/VM/edition gates
  and no delegation/chats/automation/OneNote. Next heap lifecycle/reset still
  requires owner continuation. Earlier entries retain their dated scope.

- Completed 2026-10-14: SPEC-0029 / IMPL-0033 / TEST-0037 bounded hosted
  typed Toolbox slice; SRC-0141 and docs/development/toolbox-evidence.md record
  corrected-source CI and matching68000 manifests. Thirteen prior plus four
  concurrent research originals remain unchanged/untracked. No loader/reset/
  manager/native application/boot/VM step follows from this completion record.
  Next proposed contract is heap lifecycle/reset, subject to owner continuation;
  all existing source/role/owner/edition gates persist.

- On 2026-10-13 the owner said "OK, Claude is working on it. Now, do your next
  task" (SRC-0139) after the typed Toolbox argument/completion recommendation.
  SPEC-0029 / ADR-0033 covers the original bounded hosted dispatcher, staged
  output/result writes, bounded returning/auto-pop completion and root-only
  ExitToShell binding, original conformance/compiler/CI and ordinary Git/HQ/wiki
  evidence. Continuation and register-format exceptions are typed refusals;
  stack guest patches/direct veneers/manager callbacks remain deferred. No
  loader, heap reset, QuickDraw/managers, boot binding, native application/SDK,
  VM/emulator/reference run or UART body work. Preserve all thirteen research
  originals and concurrent Claude re-entry drafts. No delegation/chats/automation/
  OneNote; standing source/role/owner/edition gates remain. This supersedes the
  preceding intake-only limit solely for this bounded dispatcher slice.

- On 2026-10-13 the owner requested "Lead intake review: Claude heap-lifecycle
  and dispatch research" (SRC-0135..0138). Verify five identities, preserve all
  thirteen unchanged/untracked originals and byte-identical copies/manifest
  outside Git; no reset/stash/clean/pull. Review published source images and
  record HL/HT/convention/profile dispositions in one lead review. HL-Q1..20
  remain open; HT cases are unexecuted. This is ordinary Git/HQ/wiki intake
  only: no loader, heap reset, Toolbox binding or other implementation without
  separate owner approval. All source/role/native/VM/edition gates and no
  delegation/chats/automation/OneNote persist. Earlier permissions below are
  dated completed slices, not authority to implement this intake's proposals.

- On 2026-10-13 the owner said "OK, Claude is researching this. Move on to
  the next task" (SRC-0132) after the bounded-session recommendation. Finalize
  SPEC-0028 / ADR-0032 and implement/verify the original hosted OS/core session,
  trusted exit boundary and shared budgets with normal compiler/CI/Git/HQ/wiki
  evidence. No loader/heap reset/actual Toolbox completion binding/boot entry,
  native application/SDK, VM/emulator/reference run, QuickDraw or UART body work.
  Preserve all research originals and concurrent Claude heap drafts; existing
  source/role/owner/edition gates and no delegation/chats/automation/OneNote.
- The owner requires every research request for another tool to be presented
  as a complete copyable prompt (SRC-0131), preserving existing scope and gates.

- On 2026-10-13 the owner requested a separate launch/segment research intake
  (SRC-0129/0130) after finishing and recording the resource reader. Preserve
  eight research originals and byte-identical intake copies outside Git;
  verify specified published citations and record LK dispositions/prerequisites.
  LQ-1..15/AL-2 remain open, LO-1..8 are unauthorized, cases are unexecuted,
  and no fixture/code, implementation or VM/emulator/reference run is authorized
  by this intake. Existing native/source/role/edition gates and no delegation,
  chats, automation or OneNote persist. Normal Git/HQ/wiki research records.

Read `docs/clean-room/POLICY.md`, `CONTRIBUTING.md`, and
`provenance/README.md` before researching or changing this project.

- On 2026-10-13 the owner said "OK, continue with the next task" after the
  proposed resource-fork reader (SRC-0126). Implement finalized SPEC-0024 /
  ADR-0028, execute TEST-0032 using original generators/model/boundary fixtures,
  and maintain compiler/CI/Git/HQ/wiki evidence. No Resource Manager loading,
  MFS decoder, application execution, boot binding, VM/emulator/reference run,
  native application/SDK, QuickDraw or UART body work. Preserve all research
  originals and concurrent Claude launch/segment drafts. No delegation, new
  chats, automation or OneNote; all source/role/owner/edition gates remain.

- On 2026-10-13 the owner said "Continue" after bounded guest calls and the
  proposal to connect guest trap patches and calling rules (SRC-0121).
  SPEC-0027 / ADR-0031 authorizes a separate opt-in typed register patch executor,
  original probes/tests, compiler/CI and ordinary Git/HQ/wiki evidence. This
  supersedes the preceding slice's no-guest-patch-installation limit only within
  this bounded hosted profile. Preserve all six research originals; no new boot
  binding, VM/emulator/reference trial, native application/SDK, QuickDraw, UART
  body work, delegation, new chats, automation or OneNote. All standing gates.

- On 2026-10-13 the owner said "Consider these items and then move on to the
  next thing" after supplying Claude's QuickDraw draft (SRC-0116/0117).
  SPEC-0026 / ADR-0030 covers checked JSR/JMP and a bounded ordinary returning
  guest-call service, original hosted probes/compiler/CI checks and ordinary
  Git/HQ/wiki evidence. Preserve all six untracked research drafts. No QuickDraw
  implementation, guest-patch installation, new boot-entry binding, emulator/VM
  run or native application/SDK. Published-only source and UART role boundaries,
  no delegation/new chats/automation/OneNote, and all edition gates persist.

- On 2026-10-13 the owner supplied Claude's callback, first-application and
  January-baseline audit update (SRC-0113/0114/0115). The lead review settles
  K-1..7 and FA-1..5 as future planning choices, with documented exceptions and
  mandatory bounded-executor safety. Original callback/resource-fork drafts
  remain unchanged and untracked; no registry/code/test/boot change or new
  implementation permission. Record ordinary Git/HQ/wiki intake only. All
  existing source/role/native-application/VM/edition gates remain.

- On 2026-10-13 the owner said "OK, move forward" after the proposed built-in
  Memory Manager trap integration and original guest program (SRC-0110).
  SPEC-0025 covers signature-checked identities, explicit CPU data addressing,
  transactional guest-trap completion, original tests/compiler audits and normal
  Git/HQ/wiki evidence. No boot-entry integration or VM/emulator run; native
  application gate stays closed. Claude's callback catalog and all research
  drafts remain untracked inputs for later review. No guest callbacks/JSR/JMP,
  UART body work, delegation, new chats, automation or OneNote in this slice.

- On 2026-10-13 the owner supplied Gemini's filename follow-up (SRC-0109).
  Review verifies published facts and records limited future-reader choices;
  supplied instructions/API/readiness claims are research, not new authority.
  No MFS implementation, historical comparison or native/VM permission inferred.
  Existing guest-trap dependency, source/role/owner/edition gates remain.

- On 2026-10-13 the owner continued review of Claude's resource-fork drafts,
  transcript and asset dossier (SRC-0106/0107). SPEC-0024 / ADR-0028 finalizes
  the original bounded reader contract; TEST-0032 is planned, not executed.
  Original research copies remain unchanged and untracked. Asset tracking is
  planning under existing policy; attachment recommendations are not automatic
  instructions or new approval gates. No parser/asset/boot implementation or
  VM/native-application permission is inferred; prior compiled results and all
  source/role/owner/edition gates remain unchanged. Normal Git/HQ/wiki records.

- On 2026-10-12 the owner said "Do it" after the checked-memory checkpoint
  and proposed Memory Manager finalization/allocation (SRC-0101). SPEC-0023
  covers original bounded hosted pointer/handle/size/lock/copy services, reviewed
  register contracts, interface records, tests/compiler audits and normal Git/HQ/
  wiki evidence. No boot-entry integration or VM/emulator launch is included.
  Preserve the closed native-application gate, UART role separation and all
  historical/physical/edition limits. Claude's drafts retain their research role.

- On 2026-10-12 the owner confirmed Claude's native-application handoff and
  directed recording its decisions (SRC-0099). Transparent variants in the
  ordinary experience are the future requirement; GPL-3.0-or-later stays
  unchanged. No x86 application, native application test, native image loader,
  converter/packager, native header/SDK or native-entry implementation is built,
  loaded or run until the owner explicitly confirms the 68k experience complete.
  Own native OS/core/tools/hosted conformance are unaffected. Proceed only with
  the 68k interface registry, JSR/JMP/call-into-guest, built-in/guest typed entries
  and CODE-only launch selection through normal contracts. Obtain the owner's
  go-ahead before any future emulator/VM run; earlier continuation entries do
  not override that gate. No delegation/chats/automation/OneNote. See ADR-0026
  and docs/development/m68k-runtime-plan.md.

- On 2026-10-12 the owner said "OK, Claude is researching. Now, get to work"
  after assigning Claude Memory Manager research only (SRC-0096). SPEC-0022
  covers checked guest memory and bounded data/control instructions, original
  fixtures/tests, compiler verification and ordinary Git/HQ/wiki evidence.
  Preserve Claude's two research drafts, UART role separation and existing
  exact-image/platform/edition gates. No new delegation or OneNote.

- On 2026-10-12 the owner said "continue" after the shared services checkpoint
  and proposed real instruction-call boundary (SRC-0092). SPEC-0021 covers an
  original bounded 68000 interpreter/exception bridge, common original program,
  tests/native builds, checks on the already cleared platforms with fresh
  isolation, and ordinary evidence/HQ/wiki Git publication. General CPU/historical
  gateway remains open. No UART body work, delegation, Apple input or OneNote.

- On 2026-10-12 the owner said "OK, let's do it" to the shared OS services
  step after the PC fault checkpoint (SRC-0088). SPEC-0020 implements the first
  bounded portable trap-address service slice, its own native build probes,
  verification on the already cleared VirtualBox/Snow platforms with fresh
  isolation, and ordinary evidence/HQ Git publication. SPEC-0017's complete
  historical gateway remains a draft. Preserve UART role separation and the
  existing black-box/physical/edition boundaries; no new delegation or OneNote.

- On 2026-10-12 the owner said "OK, do it" to the proposed short PC controlled
  CPU-fault checkpoint (SRC-0085). SPEC-0019 covers original exception/probe
  assembly, image audits, a new isolated VirtualBox VM, repeated headless cold
  starts, post-readiness observation of our terminal registers, and ordinary
  evidence/HQ Git publication. Preserve existing VMs, SRC-0059's black-box
  boundary, UART role separation, B2 criterion 6 and all Mac/edition gates.
  No firmware memory extraction, new delegation, OneNote or physical writing.

- On 2026-10-11 the owner reaffirmed independent PC/Mac ports without an Apple
  supplied ROM, delegated the practical route, allowed other emulators, then
  said "OK, do it!" after the proposal to build our replacement ROM and test
  it in Snow (SRC-0076). This supersedes the earlier Mini-vMac-only availability
  assumption. Continue strictly from eligible published manuals after the
  recorded discovery audit; no uncertain excerpts or emulator source are inputs.
  No delegation or emulator fork is authorized. Exact black-box platform
  clearance precedes launch; retain the existing physical/128K/edition gates.

- On 2026-10-11 the owner made booting in Mini vMac the top priority, "regardless
  of the size" (SRC-0073), and handed the work to ChatGPT. Only Mini vMac is
  available; no real Macintosh. Size budgets are deferred, not dropped. Clear
  Mini vMac as a black-box test platform (as SRC-0059 did for VirtualBox) and get
  the owner's go-ahead before running it; never use an Apple ROM for our boot
  or read emulator source.

- On 2026-10-11 the owner ruled "published editions only" (SRC-0067). Apple's
  1984–85 pre-release drafts (1984 *Inside Macintosh* binders, the February 1984
  assembly-language draft, the February 1985 hardware draft) are excluded
  (SRC-0070): they are marked "CONFIDENTIAL" and say their distribution "does
  not constitute publication". Use the March 1985 Promotional Edition (SRC-0068),
  the owner's 1985 *Inside Macintosh* Volumes I–III (SRC-0071; PDF kept outside
  Git, never committed) and Motorola's manual (SRC-0069). Facts in the
  owner's 2026-10-07 research dossier that came from the drafts must be
  re-sourced first. See [System 1 sources](docs/research/system1-sources.md).
  The VM trial lane is complete.

- On 2026-10-11 the owner answered "Go ahead" to the SPEC-0016 VirtualBox
  trial (SRC-0065): fresh pinned Windows verification, one new isolated project
  VM, three headless cold starts and a separate owner-operated Space session,
  retained observations and ordinary evidence/HQ Git publication. Preserve all
  existing VMs and SRC-0059's black-box boundary. Criterion 6 is unchanged;
  firmware self-report is not an image hash. No new guest/driver implementation,
  delegation, other emulator clearance, automation, physical writing or OneNote.

- On 2026-10-08 the owner delegated the B2 qualification decisions to Claude
  ("I think you should decide", SRC-0064). SPEC-0016 builds the measurements
  (firmware self-report, controller output port, bit-20 alias probe) into the
  normal native image. Criterion 6 stays as written: never relabel the firmware
  container hash as an image hash. Running the new image in a VM still needs the
  owner's explicit go-ahead in chat; QEMU/OVMF/MAME are not cleared platforms.

- On 2026-10-08 the owner handed leadership back to Claude after the VM trial
  (SRC-0063) for a read-only formal B2 audit (TEST-0023).

- On 2026-10-08 the owner returned leadership through the boot-media handoff
  and explicitly answered "Yes—prepare and run the isolated trial" (SRC-0060).
  SPEC-0015 covers a fresh isolated VirtualBox machine, owned SPEC-0014 media,
  black-box observation/repeated cold starts and ordinary publication/HQ Git
  planning. SRC-0059's no-firmware-internals boundary persists. Existing VMs,
  physical disks, delegation, new chats, automation and OneNote remain excluded.

- On 2026-10-08 the owner said "ok, do it." to the media and documents-only
  firmware tracks (SRC-0056). Claude leads this work; ordinary spec/code/tests,
  provenance, commits/pushes and HQ registration. The owner accepted Microsoft's
  FAT32 specification terms for PC boot tooling, outside the System 0.97
  clean-room scope. VirtualBox 7.2.16 EFI is cleared only as a black-box test
  platform (SRC-0059): never extract or study its internals. No VM trial without
  a separate owner go-ahead; no delegation or OneNote.

- On 2026-10-08 the owner explicitly authorized exactly one fresh implementation
  agent for SPEC-0013 polling UART diagnostics (SRC-0053), including original
  spec/code/tests/provenance and an unpushed ordinary commit. The coordinating
  lead handles HQ/publication; no further delegation, chats, automation, OneNote,
  native/VM execution or firmware eligibility waiver. Macintosh goals persist.

- On 2026-10-08 the owner returned implementation leadership to Codex with the
  Claude continuation handoff (SRC-0048). Continue SPEC-0012's reviewed PS/2
  contract, implementation, tests, provenance and ordinary commits/pushes.
  No delegation, additional chats, automation or live OneNote publication.
  Firmware eligibility and observed execution precede native boot claims;
  preserve the independent real Macintosh/mini vMac and three-edition goals.

- Preserve the exact human-facing name **ClasSICK 0.97**.
- Behavioral compatibility is the goal; Apple's implementation is not a source.
- Never fetch, inspect, copy, disassemble, decompile, translate, or prompt with
  Apple executable code, ROMs, leaked source, or prohibited derived material.
- Uncertain provenance makes a source ineligible until reviewed. Record source
  identifiers and document edition/page/section when supporting behavior.
- Keep historical facts, observations, design decisions, and implementations
  explicitly distinguished. Later Macintosh manuals do not prove 1984 behavior.
- Core code must be freestanding, architecture-neutral C. Keep guest addresses
  distinct from native pointers and hardware in platform/architecture layers.
- Implement from approved behavioral specifications; each compatibility change
  needs provenance, tests, and an explicit compatibility status.
- Use only independently generated, rights-cleared public fixtures. Private
  reference materials belong outside the repository and ordinary CI.
- The owner approved GPL-3.0-or-later on 2026-10-07 for project-owned code,
  documentation and assets. Preserve that license and inbound-equals-outbound
  contribution terms; changes require explicit owner approval. Third-party inputs
  still need separate license/provenance review.
- The founding assignment covered documentation, policy, tooling, and build probes.
  On 2026-10-07 the owner authorized the project implementation chat to finalize,
  implement, test and publish SPEC-0001 bounded portable graphics surfaces. This
  supersedes the bootstrap-only restriction for that subsystem. Subsequent
  milestones remain separate; surfaces do not complete M0.1 or any boot edition.
- The owner then authorized moving on to the Windows presentation adapter and
  original visible test scene. Finalize SPEC-0002, implement and verify that scoped
  milestone, and maintain/publish its evidence using the existing project workflow.
  Arenas, normalized input, clocks and boot work remain subsequent contracts.
- The owner reaffirmed the independent clean-room Mac boot goal (real Macintosh
  and mini vMac validation), then instructed this chat to continue on 2026-10-07.
  This authorizes SPEC-0003 bounded core arenas, viewer-buffer integration, tests,
  provenance and normal progress publication. Input/clock and boot remain later
  contracts. Emulator validation does not close the physical-hardware edition gate.
- The owner then instructed this chat to move to the next step on 2026-10-07.
  This authorizes SPEC-0004 bounded normalized keyboard events, Windows/viewer
  integration, independent tests and ordinary evidence/Git wiki planning. Timing,
  broader input and boot remain subsequent work; B1 remains open.
- The owner authorized moving on to the portable clock on 2026-10-07 and named
  their installed VirtualBox for future boot tests. Finalize/implement/verify
  SPEC-0005 and hosted timed behavior, then publish evidence and Git wiki plans.
  Record VirtualBox availability without claiming a VM boot or hardware result.
  Debugger verification and native boot remain separate gates.
- The owner then said "Do it" after the proposed debugger checkpoint. This
  authorizes hosted LLDB launch/source/state/step/resume verification, necessary
  viewer launch fixes, repeatable checks, provenance/publication and Git wiki plans.
  TEST-0009 / IMPL-0007 are development evidence; B1 acceptance and native boots
  remain separate. Live OneNote still requires an explicit publication request.
- The owner authorized the remaining B1/M0.1 audit with "OK, do it", then
  explicitly requested opening the keyboard validation viewer on 2026-10-08.
  TEST-0010 / IMPL-0008 combine hosted startup/input/timing/shutdown checks;
  the operator session passed two Space presses/releases and Escape. B1/M0.1
  technical acceptance is now passed, with human provenance review separate.
  Native linkage/runtime, a second compiler and all OS boots remain later gates.
- The owner next authorized standalone linking and a second compiler with "Do it"
  on 2026-10-08. ADR-0009 / IMPL-0009 / TEST-0011 cover current-core no-runtime
  PE link fixtures and scoped GCC core conformance, including pinned portable
  tooling and normal evidence/Git wiki publication. These fixtures are never
  loaded; firmware/loader/stack/runtime beyond this profile and B2 remain separate.
- The owner then said "OK, move on" on 2026-10-08 after the proposed firmware
  handoff/device contracts. SPEC-0006 / ADR-0010 / IMPL-0010 / TEST-0012 cover
  original preboot validators/exit model and proposed PC ownership/device profile,
  hosted checks and normal evidence/Git wiki plans. Actual UEFI calls/native entry,
  drivers and boots remain later implementations. Uncertain firmware is ineligible
  until exact provenance is reviewed; do not adopt it from product/license names.
- The owner accepted the next firmware review/native-loader step with "OK, do it"
  on 2026-10-08. SPEC-0007 / ADR-0011 / IMPL-0011 / TEST-0013 cover original x64
  UEFI calls, owned-stack stop scaffold, hosted mock checks, unloaded EFI image
  audits and ordinary evidence/Git wiki publication. Firmware review is limited to
  exact publisher build/license metadata while provenance remains uncertain. Do
  not adopt/run firmware until reviewed; native exceptions/devices/B2 and all
  edition boots remain separate. Preserve real Macintosh/mini vMac goals.
- The owner said "ok, let's do it" after the first independent PC-boot milestone
  was explained on 2026-10-08. This authorizes progressing its missing startup
  components, including SPEC-0008 owned x64 exception state, original implementation,
  tests and normal evidence/Git wiki publication. Exact firmware eligibility and
  observed native execution remain prerequisites to boot claims; no provenance
  waiver, live OneNote instruction, delegation or change to Macintosh goals.
- The owner handed implementation leadership to Claude on 2026-10-08 (SRC-0044),
  authorizing the missing B2 boot components, their specifications, code, tests,
  provenance, documentation, commits and pushes, then answered the verification
  approach with "Do whatever you feel is best". SPEC-0009 / ADR-0013 / IMPL-0013 /
  TEST-0015 add original native framebuffer presentation and a boot scene, with
  hosted tests and unloaded EFI linkage. Develop in the lead's Linux workspace if
  useful, but run evidence gates with the pinned Windows tools. No firmware waiver,
  delegation, live OneNote instruction or change to the Macintosh goals.
- The owner then said "Do it" (SRC-0045) for the next named step: SPEC-0010 /
  ADR-0014 / IMPL-0014 / TEST-0016 ACPI RSDP capture, a bounded XSDT/FADT walk,
  PM timer extension and a gated native probe. Hosted/unloaded verification only.
- The owner said "do it." (SRC-0047) for SPEC-0011 / ADR-0015 / IMPL-0015 /
  TEST-0017: one cooperative 60-second progress loop on the PM timer in the
  native stop path. Hosted/unloaded verification only.
- Use `C:\Repos\ClasSICK-0.97` for source and `C:\ClasSICK` for local build/test
  output on the founding workstation. Do not depend on those paths in core code.
- Do not create specialist chats or delegate work merely because the roadmap
  suggests future roles. Create them only when the user requests them.
- Keep bootstrap status honest: build probes are not an OS boot or compatibility
  proof. Update ADRs and evidence when a claim becomes verified.
- Cross-project evidence and documentation are maintained in the owner's separate
  `C:\Repos\ClasSICK` headquarters repository. Consult its current instructions
  before cross-project changes; keep private headquarters content out of this
  public repository. Its registry preserves this project's local record namespace.
- The owner's latest HQ instruction supersedes automatic OneNote maintenance:
  ordinary progress updates publish wiki plans in the private HQ Git queue.
  Use repository mirrors without OneNote discovery, reads, writes or retries.
  Contact OneNote only on "update wiki" or a specific requested OneNote operation;
  each request authorizes one pass under the OneNote skill and HQ wiki protocol.
  Preserve personal notes and verify writes in that pass; leave unfinished scope
  queued. Never commit personal notebook IDs, URLs or reference payloads here.
