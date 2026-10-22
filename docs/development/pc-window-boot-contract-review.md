# PC window boot-binding lead review

2026-10-22;SRC-0346..0348;SPEC-0059;ADR-0068;TEST-0070.
Behavior contract finalized;implementation absent and not authorized by this slice.
Baseline `3cf2b723c9388890a7cc2808f948abb518b0935e`. Existing qualifications reused;no source search
or manual/UART exposure. One Grok primary returned the usable contract/10 proposals/
28 cases;Codex integrates and makes the explicit corrections below.

| Proposal | Disposition | Lead decision |
| --- | --- | --- |
| B-neutral-split | adopt with changes | Full UEFI ownership/mapping/capability gates,not exit alone;PC session/bridge own no UEFI types. |
| B-row-present | adopt | Borrow fixed Mono canvas and one RGBA row;reuse framebuffer APIs unchanged. |
| B-margin-once | adopt with changes | Initial full physical row publication;later only logical canvas;software completion is not atomic scanout. |
| B-arena-once | adopt with changes | Hosted recipe has pixels=NULL;54704 fixed buffers at max width,plus64 if art bits in RAM;no double1024 allocation or whole-fit claim. |
| B-neutral-cancel | adopt with changes | Begin+cancel before ONE first publication;software neutral state is not device evidence. Require released motion before qualification clicks. |
| B-one-poller | adopt with changes | Clear receipt before clock;poll once when eligible;drain8 before mouse dispatch. Terminal error cannot replay a receipt. |
| B-publish-accept | adopt with changes | Every accepted RUNNING call publishes;startup is one batch. Owner pre-effect refusal can still terminally diagnose the session. |
| B-fault-class | adopt with changes | Distinguish owner prefix,presenter refusal and unknown hardware exception;dedicated versioned trace semantics,not old UART/keyboard ABI. |
| B-uart-bypass | adopt | No UART body/observer change;explicitly disclose absent telemetry. |
| B-profile-opt-in | adopt with changes | Future WindowDrawing/CLASSICK_WINDOW_DRAWING;preserve both timed old modes AND already-continuous outline behavior. |

The fixed buffer calculation is21888+48+4W (54704 at W8192);art bits,state,
alignment,trace,queue/call stacks and linked-image closure remain separate costs.
Independent code checks confirm framebuffer presenter rejects Mono1 directly,
hosted product.pixels isNULL,and pointer_cancel sets suppression without moving
or counting. No code/test execution is needed to admit this behavioral contract.
The worker's old-preview Escape claim is corrected against SPEC-0050/current
keyboard compile guards. Its nine-record capacity8 case is corrected;four
additional planned refusal/publication/terminal cases bring the inventory to32.

Open implementation decisions:freeze native signatures/checked workspace layout
and exact versioned trace encoding before coding/candidate/parser claims;measure
all costs/compiler imports and publication latency. These are implementation
details,not unknown historical behavior. Mac M1-M7,PC GUI/human/CPUID,source/role/
UART/historical/edition/physical/full128K/B2/native-app/Linux/mini-vMac/both System1
directions stay open. BIOS remains a separate long-term goal;no payload ABI chosen.

One Grok4.7 low call,reported grok-4.7-build,118.48s,45866-byte/652-line frozen
public input;73 artifact lines/1659 words within220/2400 caps. Zero observed
tools,writes or tests;no retry,second provider or requalification. CLI warned
preferred-model fallback;one completed turn. Provider reports28695 total tokens
and$0.02910468 session/list cost;not a purchase,balance or measured Codex savings.
All-four remaining allowances unavailable. Raw captures outside Git.

Next accepted-code proposal:isolated Mono1/one-row presentation bridge,then
neutral session,then opt-in UEFI binding. This intake performs documentation
and preservation checks only;32 cases stay unexecuted,no fixtures/header/code/
build/target trial/release. Current released assets/tags and24 originals/held
brief/old rows/checkpoints/retired wiki preserved.

[Contract](../specifications/SPEC-0059-pc-window-boot-binding.md),
[cases](../../provenance/records/TEST-0070-pc-window-boot-plan.md),
[input identities/receipt](pc-window-boot-contract-review.json),
[worker prompt](tasks/pc-window-boot-contract-grok.md).
