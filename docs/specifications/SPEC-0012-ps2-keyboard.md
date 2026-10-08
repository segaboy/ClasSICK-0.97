# SPEC-0012 — Bounded polling PS/2 keyboard and interactive native loop

Finalized v1, 2026-10-08, implementation-lead self-review; human provenance
review pending. Owner SRC-0048; interfaces SRC-0049–0052; ADR-0016 /
IMPL-0016 / TEST-0018. Original PC contract, no historical Macintosh behavior.

## Profile and source boundary

One polling x64 CPU, interrupts masked, identity mappings and owned descriptors
as SPEC-0008. Require a PS/2-mode compatible controller and untranslated set-2
keyboard. ACPI must positively declare the controller. No USB legacy emulation,
SMM interference or competing controller consumer; startup qualification must
retain A20 and required machine state during controller self-test. Published
interfaces describe hardware contracts, not proof that VirtualBox implements them.
No firmware callbacks, runtime library, external driver/header or implementation.

Source scope: SMSC defines command AA, while Holtek HT6542B documents 55 for
its power-on self-test. Requiring 55 after our AA is an explicit compatibility
qualification criterion; these documents do not prove every controller's reply.

`cs_acpi_8042(read, context, fadt, out)` revalidates a FACP table (length
116..65536, checksum, revision >=3), reads IAPC_BOOT_ARCH at 109 and emits
bit 1 as 0/1. Existing ACPI result codes and output stability apply. Old revisions
return UNSUPPORTED. The interactive loop rejects absent/unsupported indications
before port I/O or framebuffer writes. Other FADT flags are outside this function.

## State machine

Caller owns live disjoint state, callbacks/context and SPEC-0004 queue. Callback
read/write addresses are solely 0x64 (status/command) and 0x60 (data). These
callbacks denote raw hardware in native use and authored responses in tests.
Begin initializes every state field; poll receives monotonic extended PM ticks.
Each poll reads at most one status and one data byte OR writes one command/data
byte; no internal waits. Writes require IBF=0; reads require OBF=1.
Status bit 5 distinguishes auxiliary bytes; bits 6/7 indicate transport errors.

Startup sequence (one operation per polling turn):

1. Disable keyboard (AD) and auxiliary (A7); drain at most 64 stale bytes.
2. Read configuration (20); retain only system flag bit 2. Write (60 then data)
   system flag plus 0x30: both interfaces disabled, IRQ bits and translation clear.
3. Controller self-test (AA), require 55. Disable both interfaces again, drain,
   rewrite configuration, read it back and require bits selected by 0x73 = 0x30.
4. Keyboard interface test (AB), require 00; enable keyboard interface (AE).
5. Keyboard reset FF, require ACK FA then BAT AA; F5/ACK disables scanning;
   F0/ACK and 02/ACK select set 2; F4/ACK enables scanning. Auxiliary stays disabled.

Each keyboard byte is sent at most three times: FE requests repeat of precisely
that byte, with a maximum of two retries. Controller replies are never treated
as keyboard ACKs. Unexpected non-auxiliary responses stop with PROTOCOL; FC/FD,
00/FF where not expected stop with DEVICE; parity/timeout stops with CONTROLLER.
Auxiliary replies are consumed and counted without extending the deadline.
Startup requires keys released; concurrent startup scan bytes fail explicitly.

Each phase is bounded by 1,000,000 polling calls and elapsed PM ticks: 357955
(about 100 ms) normally, 1789773 for controller self-test (about 500 ms), 3579545
for BAT (1 s). Elapsed >= limit fails before further I/O. Begin's first poll
establishes the initial deadline; every transition establishes the next deadline.
FE retry establishes a new send phase, but the three-send cap remains. A clock
regression returns STATE with no I/O. Failed state remains terminal until begin;
READY has no timeout. A drain of 64 bytes followed by another full buffer fails.

Results: ACTIVE=0, READY=1, ARGUMENT=2, STATE=3, TIMEOUT=4, CONTROLLER=5,
DEVICE=6, PROTOCOL=7, QUEUE=8. Null arguments return ARGUMENT without mutation.
State corruption is outside live initialized-state preconditions; invalid phase
or result is detected. Failure records its result without cleanup writes, since
controller availability cannot be assumed. The native caller terminates on error.

## Set-2 events

READY polls at most one byte. Ordinary 29/76 normalize Space/Escape, source=3.
F0 introduces release; E0 suppresses an extended key (including E0 F0 suffix);
E1 suppresses the following seven Pause bytes. Prefixes span polls. Unknown keys
are ignored. Repeated make emits PRESS/repeat=1; initial make repeat=0. Release
emits only for a held key. A dropped queue event still updates physical held state.
Queue FULL increments dropped count; other queue errors terminate with QUEUE.
All diagnostic counters saturate at UINT32_MAX. Auxiliary bytes do not disturb
prefix state. Parity/timeout or 00/FF/AA/FC/FD/FA/FE in READY clears prefix/held
state and counts an error, without fabricating releases or issuing commands.
This is a two-key demonstration driver, not text input or full keyboard support.

## Interactive loop and evidence

`cs_native_keyboard_loop` is a separate successor to SPEC-0011; its timed-only
function and tests remain. It validates arguments, exit/status, ready marker,
memory map, timer, positive 8042 declaration, framebuffer and arena before device
access. It prepares original scene staging and an eight-record core FIFO from the
128-KiB arena, then runs keyboard startup while sampling the timer every turn.
Only after READY does the 1..3600-second progress duration start. Timer value,
stall and late-sample rules match SPEC-0011; keyboard work is one poll per turn.

Space initial presses advance an independent offset modulo 17. The displayed
scene is `(timed_step + offset) % 17`; repeats/releases do not change it. Escape
initial press draws `(timed_step + offset + 1) % 17`, records ESCAPED and stops.
An early Escape is not a 60-second B2 pass. A duration completes at timed step
16 while retaining the input offset. Drain at most eight queued events per turn;
no firmware or interrupt scheduling. If keyboard startup fails, return before
drawing; if it fails later, retain the last successfully drawn frame.

The caller provides 112 live trace bytes at bundle trace offset 128. Exactly those
bytes are little endian: magic 0x314B5043 at 0, version 1 at 4, result 8,
keyboard result 12, phase 16, seconds 20, elapsed seconds/ns 24/28, frames 32,
Space presses 36, Escape presses 40, consumed events 44, dropped 48, errors 52,
auxiliary bytes 56, drained bytes 60, resends 64, late samples 68, max delta 72,
display step 76, held mask 80, last key/action/sequence 84/88/92, timer reads u64
96, last raw keyboard byte 104, configuration 108. Record is rewritten on each
successful frame and on every termination; no atomic/crash-consistency claim.
Old presentation/probe records remain; offset 80's timed-only record is unused
by the new native entry. Remaining trace storage stays unchanged.

Loop results: OK=0, ARGUMENT=1, NOT_EXITED=2, NOT_READY=3, TIMER=4, TARGET=5,
ARENA=6, DRAW=7, VALUE=8, STALLED=9, NO_8042=10, KEYBOARD=11, ESCAPED=12.
In-progress frame snapshots use RUNNING=13.
Null trace writes nothing; other rejected gates write initialized diagnostics
(including the requested duration). Hardware
effects and actual latency remain unobserved until eligible native execution.

## Acceptance

Authored controller transcripts check every write, readiness constraint, response
phase, retry byte, drain/timeout/clock bound and absence of I/O after failure.
Stream tests cover releases/repeats, prefixes, auxiliary/error contamination,
unknown keys, queue overflow and counter saturation with independent expected
events. Hosted interactive tests use real core FIFO, presenter, scene and timer
extension against synthetic ACPI/ports; compare frames with the scene oracle,
check gates, Escape, duration, retained padding/guards and exact trace bounds.
Both compilers, actual i686 portable driver tests, x64 ASan/UBSan, object audits
and unloaded O0/O2 EFI twins apply. Exact original INB/OUTB bytes, corruption
controls and omitted-keyboard links reject. No real ports execute in hosted tests.
Firmware eligibility, media, UART, native timing/input/fault observations and B2
remain open. Real Macintosh/mini vMac and physical edition requirements persist.
