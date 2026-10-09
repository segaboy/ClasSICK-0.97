# SPEC-0013: Bounded polling UART diagnostics

Version 1, finalized 2026-10-08. Original PC development profile; SRC-0053/0054,
SRC-0052 byte-port instructions and SRC-0046 PM timer. ADR-0017 / IMPL-0017 /
TEST-0019. Hosted/unloaded evidence only; human provenance review pending.
Version 2, 2026-10-09 ([SPEC-0016](SPEC-0016-qualification-diagnostics.md)):
banner `UART v2`, firmware/owned-address/qualification lines, a required
qualification pointer and a 1-s / 4,000,000-turn termination drain.

## Interface facts and qualification

TI TL16C550C SLLS177I (March 1994, revised March 2021), PDF/printed 21–23,
Tables 7-1/7-3 specify byte register selection and DLAB aliases. Sections 7.7.2,
7.7.4/5 (24–25) describe FIFO reset, polling and disabled interrupts; 7.7.7/8
(27–28) specify 8-bit/no-parity/one-stop configuration, THRE, TEMT and destructive
LSR error reads. 7.7.9 (29) specifies modem outputs/autoflow/loopback controls;
7.7.11 (31) supplies the divisor equation; 7.7.13/14 (32) specify SCR and THR.
This is a qualified compatible-interface profile, not universal 16550 support.

The proposed native target must exclusively decode eight contiguous byte ports
at 0x3F8, implement these scoped TI semantics (including SCR/readback/FIFO status),
and supply a 1.8432-MHz clock. Divisor 12 requests 9600 baud, 8N1. Those mapping,
clock, device model, cable/capture, post-exit access and absence of firmware/SMM
interference assumptions remain unverified; TI does not establish VirtualBox
conformance. No discovery from arbitrary ports and no hardware/VM execution here.

## Portable transport

Caller supplies disjoint live state, a 512-byte byte queue, byte callbacks, a base
in 1..65528 and nonzero 16-bit divisor. Begin validates before mutation and sets
scalar state without reading storage or ports. No allocation/global state/runtime.
Poll receives monotonic extended PM ticks. Invalid arguments cause no mutation/I/O;
clock regression or invalid initialized state stops with STATE. Sticky failures
perform no more I/O; begin is the explicit restart. Physical callback latency is
outside the software bound and must be qualified separately.

Startup makes exactly one register operation per poll: LCR=03, IER=00, FCR=07,
MCR=03, save SCR, write/read A5, write/read 5A, restore SCR, LCR=83, write DLL/DLM,
read DLL/DLM, LCR=03, read LCR/IER/MCR, read IIR and require high bits C0. Readbacks
must match; otherwise DEVICE, with no attempted cleanup on untrusted hardware.
FIFO reset discards prior bytes. No master-reset assumption, loopback, RX service,
interrupts, DMA, modem flow control or firmware logger. Startup is bounded by
357955 ticks and 100000 calls, checked before further I/O; first poll establishes
the epoch and count. Readback does not prove hardware identity or line delivery.

READY polls read LSR once, retaining last value and accumulated error bits 1–4/7.
Any of those bits stops LINE, conservatively including incoming receive errors.
THRE permits at most one THR write, consuming exactly one queue byte. No THR
write without THRE. A write counts bytes accepted by THR, not delivered remotely.
Finish forbids further enqueue and waits for the queue to empty and TEMT to be
observed; only then DONE. Queue-empty without finish remains READY. Pending work
has a 357955-tick/100000-call no-progress limit, restarted by each THR write.
Enqueue of an entire 1..512-byte record is atomic: FULL preserves queue content
and increments saturated dropped-record/byte counters; no partial record. Records
may queue during startup; failure retains unsent data. Zero-size enqueue succeeds
without reading input. Counters saturate. State/storage content after begin is
defined only for queued bytes, with consumed bytes retained until overwritten.

Results ACTIVE=0, READY=1, DONE=2, ARGUMENT=3, STATE=4, TIMEOUT=5, DEVICE=6,
LINE=7, FULL=8. FULL is an enqueue result, not a terminal transport state.

## Native successor

Keep cs_native_keyboard_loop and its SPEC-0012 tests. Add an observed variant
that uses exactly the same keyboard/scene/duration logic. Observer notice 0 is a
valid timer sample, 1 is termination and 2 is a frame snapshot. Frame notices
enqueue output without another poll. No observer port call occurs until
all exit/owned-ready/map/ACPI/8042/framebuffer/arena and initial timer gates pass.
The UART successor supplies that observer and native entry selects it for 60 s.
Observer callbacks and owned buffers are truthful, live/disjoint; trace capacity
is the caller's responsibility as in preceding native APIs.

Diagnostics enqueue original ASCII banner, keyboard-ready, changed-frame and
termination records (v2 adds the SPEC-0016 lines). Frame line fields are fixed eight-digit uppercase hex:
`frame=`, `sec=`, `space=`; end fields `result=`, `kbd=`. CRLF framing; no format
library. Queue drops are explicit. Each timer turn runs at most one UART poll;
serial startup/stall/line failures do not prevent keyboard progress or change its
return result. Termination attempts a finite drain with fresh PM sampling,
maximum 100000 turns and 357955 elapsed ticks in v1 (v2: 4,000,000 turns and
3,579,545 ticks), without extending the keyboard duration. Fresh timer
initialization/map failure sets UART STATE; invalid drain
samples, elapsed overflow or exhausted drain bounds set TIMEOUT. The independent
drain epoch starts at its fresh initial timer sample; callback latency is external.
No asynchronous exception logger: SPEC-0008 first-fault RAM evidence remains.

## Owned trace

Existing records at offsets 0,32,80,128 remain nonoverlapping. New UART trace
starts at 256 and is exactly 640 bytes: 128-byte little-endian header plus the
512-byte transport ring. Bundle remains 512 KiB. Header u32 offsets: magic
0x31555043 at 0, version 1 at 4, keyboard return at 8, UART result at 12, phase
16, base 20, divisor 24, queued 28, head 32, THR bytes 36, dropped records 40,
dropped bytes 44, LSR 48, accumulated line bits 52, started 56, closing 60,
observer turns 64, flush turns 68, records attempted 72, final TEMT 76. u64 last
extended ticks at 80; bytes 88..127 reserved zero. Ring bytes are initialized
zero by the native wrapper and retain data until ring reuse. Rewritten header is
not atomic/crash-consistent. Rejected gates retain NOT_STARTED=9 and no ports.

Acceptance checks exact startup accesses, aliases/readbacks, THRE/TEMT distinction,
line errors, retained FIFO order, atomic overflow, bounds/clock/counter saturation,
invalid arguments and sticky failures using original fixtures. Native tests compare
actual ASCII output, keyboard scene oracle, all gates, 60-second/Space/Escape
behavior, stalled serial independence, final drain and guard bytes. Clang/GCC
x64/i686 portable tests, x64 sanitizers, endian/object audits, O0/O2 EFI twins
and new omitted-object controls apply. No native delivery/boot/parity claim.
