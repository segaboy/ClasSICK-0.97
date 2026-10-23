# Pure normalized capture inbox - actual evidence

2026-10-23;SRC-0372..0374;SPEC-0065/ADR-0074,IMPL-0066/TEST-0078.
Native68000/x64 share apps/window-scene/inbox.c. This is an isolated state
machine:producer appends full normalized records;short excluded foreground
claim/complete/discard/resume operations bracket unmasked owner work in the
**test harness**. No production foreground bridge,CPU masking/barriers,ISR,
SCC/VIA/framebuffer binding or boot input change. Existing SPEC-0063 unchanged.

PRIMING stages input while a claimed baseline is submitted. One ordinary claim
may finish after newer visible loss;no later claim until recovery. Overflow
never overwrites/discards producer data;foreground explicitly discards inbox
before external cancellation and resumes after it. First raw owner failure wins
over concurrent loss. Stop retains that cause even if its cancellation fails;
stale tokens/exhaustion cannot reopen a session. All layout/caller exclusions
and hardware facts stay separate from this sequential operation model.

One Claude Opus5.5 low call,13188B/188line original contract/type packet,
54.11s,254 C lines within330 cap,zero tools/edits/tests/retry/second worker or
requalification. Adopt code with one lead complete(no claim) STATE correction.
Worker identifies that return ambiguity;discard(pending claim) remains STATE.
Three API-derived tests plus layout witness were frozen before inbox-body
inspection. Final tests are **not unchanged author-blind artifacts**:lead adds
the exact no-claim LOSS assertion and fixes a simulated sample-failure value
leaking into restart. Removes a tautological harness mask check;no CPU exclusion
pass claimed. Real-owner test/layout remain byte-identical to initial freeze.
[Prompt](tasks/window-inbox-claude.md),[compact receipt](window-inbox-evidence.json).
Verbose captures stay outside Git:C:/ClasSICK/window-inbox-20261023-a.

Final local335 full Clang64 Debug CTests plus4 focused each GCC64 Release and
Clang ASan/UBSan total343 profile checks. Per profile:unit10024 assertions and
2400 ordered records,mock interleaving290 assertions,real owner118 assertions.
These are not that many distinct scenarios. Actual real-pointer and dragged-
window clamping reversal,loss cancellation,held-left suppression/release pass.
12 reused width/endian/EFI-ABI compiler audits pass:no imports/mutable data,
matching delayed EFI-ABI twins,per-frame512 bound. Descriptor244B on hostedx64.
Three actually compiled faulty controls are rejected;valid control passes.
Baseline-loss defect uses the interleaving oracle,not the unit-only oracle.
Windows/68000 [source-pinned CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/39951159752) passes at`dcf4f42c06873b2e189a45e9c740ef735344c12e`. Actual68000:12 objects/6 closures,matchingO0/O2/Os twins,descriptor244B,largest inbox function frame36B. All manifest input hashes match local normalized source/header bytes. No target execution.

IC-01..14 retain explicit partial/unexecuted hardware parts in TEST-0078;no
claim that all14 original cases are complete. In particular actual CPU mask
save/restore,compiler interrupt publication,non-nesting/quiescence,hardware
baseline freshness/polarity/timing and hidden physical edge loss are unproved.
M1-M7 remain partial/open;total RAM/ROM and foreground+interrupt stack still
need measurement. Existing PC GUI/human/CWP1,native-app,source/edition,physical/
full128K,B2,BIOS/Linux and both System1-ROM gates persist. No target trial,
release,new image/GIF or wiki/OneNote. Preserve24 originals,held brief,UART,
old rows/checkpoints,assets/tags. Guard/record results are not feature tests.

Reported$0.1984488 list/session cost is not a balance or measured Codex savings.
Claude's task-time allowance snapshot is private and time-limited;Grok,Copilot
and Argon High idle/current balances unavailable. No additional quota calls.
Next proposed slice:a bounded production foreground coordinator using the
reviewed short-section interface,then separately admitted CPU/SCC binding.


Repository guard passes:1086 indexed text/source files,31 registered PNGs and
1 reviewed GIF. JSON/links/Python syntax/whitespace and preservation checks pass.
Two identical push-triggered CI runs appeared;older39951159430 cancelled,newer
39951159752 retained and successful. Metadata-only follow-up changes no tested
code,input,API,oracle or compiled candidate.
