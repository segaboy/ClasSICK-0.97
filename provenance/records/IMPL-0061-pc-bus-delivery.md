# IMPL-0061 / TEST-0069 - Explicit PC controller delivery

2026-10-21;SRC-0338..0340;SPEC-0058;ADR-0066.
Original shared internal poll with optional receipt preserves the legacy entry
and bus layout. New required-output API clears stale data,emits each accepted
READY packet or known AUX loss directly,and leaves arguments/refusals as specified.
No new hardware command,decoder,counter,allocation or callback. Do not use
saturating totals as event tokens. Shared window owner remains the UI counter.

317 full Clang64 CTests plus12 sanitized/7 GCC32 focused checks pass (336 total).
Controller suite:1883 assertions/150 paired polls per run. Window suite:6166
assertions/222 complete-frame comparisons/70 reports per run,including actual
controller receipts,loss during drag,held suppression/release and saturation.
Three local baseline-source comparisons (Clang64,GCC32,GCC64) preserve original
4d8589c bus state,queues and I/O traffic. Three compiled receipt defects fail.
34 width/endian/import/global audits pass,plus2 expected native-x64 ABI
rejections.16-byte receipt layout asserted. Normal CI pending.

One Copilot Sonnet5.5 low test primary,16933-byte frozen public/original brief,
360.06s outer timeout,no artifact or adopted lines. Three transport EOF failures
and six model-call starts;one prompt read and one extra input-directory listing,
no outside content,edits or tests. Complete usage unavailable;timeout does not
prove backend cancellation or zero spending. No retry/second provider. Codex
supplies original fallback tests;one test fixture correction uses the existing
Set2 Space make/F0-break sequence. No hardware algorithm/source was introduced.

All feature observations are E1 synthetic transcripts. Reused renderer supports
frame equivalence,not independent renderer or hardware-direction evidence.
Changed PC controller source requires fresh candidates;old boot observations do
not qualify rebuilt images. No window boot/guest binding,VM/emulator/hardware/
reference run,release or new media.24 originals/held brief/UART/old rows/assets/
tags/retired wiki and all compatibility gates persist,including Mac M1-M7 and
PC GUI/human. [Receipt](../../docs/development/pc-bus-delivery-evidence.json).
