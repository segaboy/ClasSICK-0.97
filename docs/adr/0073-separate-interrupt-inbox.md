# ADR-0073 - Separate interrupt capture from rendering

2026-10-23;accepted original engineering boundary,SPEC-0064,SRC-0369..0371.
Keep verified SPEC-0063 sequential and wholly foreground-owned. A separate
16-record single-producer inbox permits a short excluded claim followed by
unmasked rendering. No helper/owner call in IRQ or under the inbox exclusion.
One accepted in-flight record may finish after new visible loss;cancel before
another claim. Explicitly discard the inbox rather than expecting helper loss
recovery to clear a different queue. PRIMING stages later records while a fresh
baseline is submitted;never trust a blind baseline-to-READY interval. First
owner fault wins;stop/quiescence/exhaustion are explicit boundaries.

This costs additional bounded storage and does not establish an ISR ABI,CPU
mask,compiler barrier,hardware sampling/timing or physical all-edge capture.
Reject whole-render masking,concurrent calls into the sequential helper,
volatile-only safety,producer clamping/coalescing and automatic replay.
Hardware access/acknowledge/polarity and the source disagreements remain open.
Next proposed implementation is a pure inbox plus deterministic interleaving
checks/native objects,not an SCC driver. No implementation or trial here.
