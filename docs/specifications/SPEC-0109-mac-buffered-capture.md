<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0109 - Buffered Mac input capture

2026-11-16. Original engineering design under SRC-0608 and the independent
implementation policy. No additional historical hardware claim. This opt-in
SHORT profile separates acquisition from decoding; normal profiles retain
their single-pass interrupt service.

The serialized IPL7 interrupt performs one SPEC-0091 collection and, for a
valid pending mask, one additional RR3 read. It appends the four packed
values plus this late pending observation to a 128-entry FIFO. Invalid
pending masks queue a fault observation without reading further hardware.
The interrupt never modifies the motion decoder or normalized inbox.

Foreground removes at most 32 captures per iteration, excluding interrupts
only around FIFO operations. It decodes them in order outside exclusion,
using the captured late RR3 for the existing clean-late-edge test. No current
device state substitutes for a delayed capture. Both normalized inbox producer
and consumer now run on the same foreground stack, so their exclusion hooks
are no-ops. The ordinary motion decoder, ordering, button handling, bounded
inbox, monotone coalescing and loss/rebaseline rules continue to apply.
Any failure/recovery SCC write is a separately serialized transaction.

FIFO overflow latches loss, increments a saturating counter once, and rejects
further captures until foreground observes it. Acquisition still acknowledges
hardware; it cannot leave a continuously pending interrupt that prevents
foreground recovery. Observing overflow discards the entire FIFO and enters
normalized input loss with reason 5, capture loss, without fabricating sample
bytes. Prior valid samples may already have been delivered. After ordinary
foreground cancellation and inbox WAIT, clear remaining raw records and take
a fresh synchronous baseline under exclusion before rearming SCC input.
No stale queued record may establish that recovery baseline.

The FIFO uses five explicit aligned word transfers per record, avoiding a
freestanding byte-copy helper inside exclusion. These volatile accesses are
not synchronization; caller exclusion is required. Queue initialization and
all diagnostic counters are explicit. MCQ1 contains tag, queued count, peak
and overflow count; it is an observation record, not a public ABI.

Qualification includes a FIFO oracle across wraparound, capacity and overflow;
deferred decode with hardware changed since capture; explicit loss and fresh
baseline; unchanged existing core profiles; actual BootHush interrupt/register
checks; full frame parity at the specified injected rates; finite memory and
stack bounds. No rate, physical polarity, real hardware, human input or release
qualification follows solely from implementation or a host test.

## Human-motion burst experiment, 2026-11-16

The M1 human trace overran the 128-entry FIFO seven times and reported
unresponsive/incomplete pointer travel. An explicit `--window-burst-capture`
build uses 512 records and removes at most 128 per foreground iteration.
The 10,260-byte FIFO moves to the existing application workspace rather than
consuming native global or stack space. The linker must still prove workspace
separation from the screen. Initialization, ordered decode, button handling,
overflow cancellation and diagnostic format are unchanged. The default
128/32 profile and frozen review image remain preserved.

This is a bounded buffering experiment, not a guarantee that a faster input
rate is sustainable. Qualification requires both capacity variants' host
contracts, the recorded human trace, loss/pending counters, pixel parity and
new human feedback. The queue must still reject deliberate excessive bursts.
