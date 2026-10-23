# Mac SCC and interrupt handoff review

2026-10-23;SRC-0369..0371. Contract/source review only,not a hardware adapter.
Baseline2571beaca19c4d861520d94d5fef50cbb60d59db. Existing sequential
[SPEC-0063](../specifications/SPEC-0063-ordered-window-capture.md) and its
[executed evidence](window-capture-evidence.md) are unchanged.

The key binding defect is conceptual:excluding interrupts across a complete
capture-helper foreground operation can exclude capture while its input owner
renders. The helper explicitly requires operation-level exclusion and has no
IRQ-safe publication API. Allowing a producer to preempt its owner call would
violate that contract. Neither volatile nor the helper's sequential tests fixes
this. Select a separate bounded IRQ inbox,short foreground copy/claim section,
then ordinary unmasked sequential-helper delivery. This adds storage and still
needs an actual CPU/compiler exclusion proof;it is not a current driver.

## Lead-only published source check

Reidentified admitted SRC-0363:Zilog *SCC/ESCC User Manual*,UM010902-0609,
June2009,[publisher](https://www.zilog.com/docs/serial/scc_escc_um.pdf),
88,330,952 bytes,394 PDF pages,SHA256
`2a13683ee0e423ee3cdf2781a4ce6b4df65c1e069a269e4234fa6964b4a3bea0`.
Reviewed printed30,40-41,140,142,160-161,176-177;PDF=printed+7.
Visually checked printed140,142,160,including page numbers and register diagrams.
Other source identities and earlier Mac/VIA findings are reused from
[SRC-0362..0365 review](mac-motion-prerequisites.md),not newly reverified here.
No application note,sample routine or printed symbolic listing was adopted.
Private manual/page renders never enter Git or worker inputs.

|Finding|Eligible locator|Disposition|
|---|---|---|
|Master enable is WR9 D3;external/status also needs WR1 D0 and individually enabled WR15 sources.|pp40,160-161;WR9 diagram visually checked|Adopt as family interface facts,not an initialization sequence.|
|WR9 is shared across both channels;channel resets affect receiver/transmitter,modem outputs,IP/IUS/enables and add four PCLK cycles beyond usual timing.|pp159-160,including diagram|Reuse shared-register fact;adopt reset side effects/timing obligation. No routine reset command selected.|
|An IUS latch is set by an actual acknowledge;software-ack support differs by part. WR9 D5 is reserved zero on NMOS.|pp40-41,160-161|Adopt;do not presume that CPU autovector handling acknowledges the SCC or that all handlers need Highest-IUS reset.|
|IUS priority,IP/source acknowledgement,master gating and CPU exception return are distinct operations.|pp40-41,160-161|Adopt distinction. Exact board acknowledge route and CPU return contract remain unbound.|
|WR0 figure gives Highest-IUS command111;prose labels it110,which it also assigns to Error Reset.|p140 diagram versus p142 prose,both visually checked|Keep a source disagreement. No command opcode selected;seek eligible corroborating part documentation before any use.|
|RR0 DCD description refers to pin state and enabled latch behavior without an explicit voltage-to-bit truth table in this checked passage.|pp176-177|Keep physical pin/RR0 polarity unresolved;do not infer inversion or equality.|

The command disagreement is present in the rendered publication,not merely OCR.
It does not block a hardware-free inbox design. The earlier III-25 versus
III-46 CPU-level/vector conflict,2.2us SCC access spacing,even-byte reads/odd-byte
writes,shared selector,VIA PB read side effects,and hidden even-edge loss
limitations remain. No new exact MMIO addresses,CPU mask or hardware recipe is
admitted. The family publication does not establish1984 chip/Snow/physical-board
equivalence or historical software behavior.

## Next acceptance boundary

[SPEC-0064](../specifications/SPEC-0064-mac-capture-handoff.md) freezes the
original software scheduling boundary;the associated ADR and planned cases
are linked there. Implementing a pure inbox and synthetic interleaving oracle is
the next proposed slice. Exact SCC access/polarity/acknowledge and CPU/compiler
exclusion binding follow separately. None of M1-M7 is declared fully closed;
M5 now has a verified sequential helper plus an unimplemented ingress contract.
M7 must include both descriptors,one claimed record,owner/workspace,framebuffer,
all foreground/interrupt stack and ROM costs. Previous248B/32B helper metrics
cannot be treated as total memory fit or capture-latency evidence.

No runtime/header/fixture/build-input change,feature-test execution,hardware,
emulator/VM/reference run,release or new visual evidence belongs to this slice.
Current PC evidence is reused,not credited as Mac IRQ evidence. Existing source,
edition,physical/full128K,B2,native-app,GUI/human,CWP1,BIOS/Linux and both
System1-ROM gates remain. Preserve24 originals,held brief,UART bodies,old
records/checkpoints,assets/tags and retired wiki. No OneNote/wiki work.

## Worker disposition and actual verification scope

One Grok4.7 low primary,resolved grok-4.7-build,127.28s from11,627 original/
reviewed-public bytes returns130lines/1965words within170/2000 caps. No tools,
edits,tests,retry,second worker or requalification. Lead adopts its separate
inbox/short-claim architecture and corrects producer clamping,missing inbox
discard,baseline staging,begin/restart semantics,unexpected helper state and
fault ordering. Individual dispositions and14 UNEXECUTED cases are in the
[receipt](mac-scc-binding-review.json). Raw transcript stays outside Git.
[Actual prompt](tasks/mac-scc-handoff-grok.md). Lead source eligibility/review is
separate;no manual text/images,HQ,held originals or implementation bodies sent.

Provider reports19,953 task tokens/$0.02652476 list/session cost,not remaining
allowance or measured Codex savings. All-four current remaining allowances are
unavailable;Claude's earlier snapshot is not a fresh balance. Other workers
idle. Required record guards and preservation checks are mechanical checks,
not executed feature cases. No new GIF is useful without new product behavior.
