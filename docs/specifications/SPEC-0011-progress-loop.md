# SPEC-0011 — Cooperative timed progress loop

Status: finalized v1, 2026-10-08; implementation-lead self-review, human provenance
review pending. Owner SRC-0047; internal SPEC-0005 and SPEC-0009–0010; ADR-0015 /
IMPL-0015 / TEST-0017. Original native-PC contract; no historical Macintosh behavior.

## Behavior

`cs_native_progress_loop(handoff, ready, devices, seconds, trace)` is a single
cooperative polling loop, run once after the SPEC-0010 probe in the native stop
path. `devices` supplies the SPEC-0010 memory/reader view, a 32-bit port reader
and context, and identity views of the framebuffer and the owned 128-KiB arena.

1. Arguments: non-null trace (otherwise return 1 without writing), handoff,
   devices and their members; `seconds` in 1..3600. Then the same gates as
   SPEC-0009/0010: EXITED phase with zero firmware status, and the ready marker.
2. It rediscovers the PM timer exactly as SPEC-0010 does (RSDP, map, walk; any
   failure is TIMER). It revalidates the framebuffer with SPEC-0006/0009 (TARGET),
   reinitializes the arena and prepares the scene staging (ARENA), then draws
   step 0 (DRAW). The SPEC-0009 presentation arena use is complete by then, so
   reinitialization reuses retired storage.
3. It initializes the counter from one port read (VALUE on out-of-width bits).
   Each later read samples the counter and converts total ticks since loop start
   to SPEC-0005 time.
4. Progress `step = floor(elapsed_seconds * 16 / seconds)` (0..15) redraws the
   whole frame only when the step changes. Once elapsed seconds reach `seconds`,
   step 16 (all sixteen segments lit) is drawn and the loop returns OK.
5. Diagnostics: every delta is compared with half the counter period. A larger
   delta counts as a late sample, meaning a wrap may have been missed, for example
   behind a slow redraw. The loop still continues. 20,000,000 consecutive zero
   deltas end with STALLED. There is no other wall-clock limit: an advancing timer
   ends the loop after `seconds`.

Results: OK 0, ARGUMENT 1, NOT_EXITED 2, NOT_READY 3, TIMER 4, TARGET 5, ARENA 6,
DRAW 7, VALUE 8, STALLED 9. No framebuffer write or port read precedes the
TARGET/ARENA checks. The 48-byte little-endian record at trace offset 80 holds:
magic `0x314C5043` at 0, version 1 at 4, result at 8, requested seconds at 12,
elapsed seconds/nanoseconds at 16/20, frames drawn at 24, late samples at 28,
port reads (u64) at 32, maximum delta at 40 and final step at 44.

## Acceptance and limitations

Hosted tests drive synthetic counters through the real presenter and scene. They
cover every gate and argument error with no port or framebuffer access. Progress
cases use 24/32-bit counters, wrapping start values and steps from 1,000 ticks to
one second per read, over 1-, 3-, 7- and 60-second durations. They check exact
frame counts, elapsed time, read counts, maximum and late deltas, and the final
step-16 frame against the independent scene oracle. Stall after exactly
20,000,000 unchanged reads and out-of-width values are also covered. The EFI image
links the loop; the native stop path runs it for 60 seconds, then halts.

This adds no keyboard, interrupt, scheduler, recovery, UART or wall-clock
comparison. A missed wrap is detected only heuristically, and a timer paused with
a stopped VM is indistinguishable from real elapsed time. Real visibility, duration
against an independent clock, B2, Mac/mini vMac and the physical-edition gates
remain unverified.
