# SPEC-0005 — Portable monotonic time and deterministic clock

Status: finalized v1, 2026-10-07, project lead self-review under SRC-0023.
Independent generic contract, IMPL-0006 / TEST-0008. Host interface prose SRC-0024.
No historical TickCount, calendar, scheduler, event timestamps or native driver claim.

## Representation and ownership

cs_time is two uint32_t fields: seconds and nanoseconds. A canonical value has
nanoseconds < 1,000,000,000. Seconds range 0..UINT32_MAX; overflow rejects rather
than wrapping. Nanoseconds are representation units, not an accuracy guarantee.
Values/durations are compared only within one caller-declared domain and epoch.
No UTC, timezone, implicit epoch, hardware-counter frequency or sleep policy exists.

cs_clock contains one canonical last sample. The caller owns/synchronizes the
descriptor and its lifetime. init/reinit establishes an epoch and invalidates
comparisons with older samples/deadlines. Metadata checks do not identify clocks
or enforce domain/lifetime ownership. Functions use value inputs, and output must
be a truthful live writable object; core code has no host calls, heap, global data,
floating point, 64-bit arithmetic/division or mandatory compiler helpers.

## Operations and precedence

Results: OK=0, ARGUMENT=1, STATE=2, VALUE=3, BACKWARD=4, OVERFLOW=5.
Every rejection preserves descriptor/output fields. Constant bounded work.

- cs_time_add(base,duration,out): null output -> ARGUMENT; invalid base/duration
  -> VALUE; seconds addition including nanosecond carry beyond UINT32_MAX ->
  OVERFLOW. Success writes canonical sum.
- cs_time_elapsed(start,end,out): null output -> ARGUMENT; invalid inputs -> VALUE;
  end before start -> BACKWARD. Success writes canonical difference, including
  borrowing a second when needed.
- cs_time_reached(now,deadline,out): null output -> ARGUMENT; invalid inputs ->
  VALUE. Success writes exactly 1 when now >= deadline, otherwise 0.
- cs_clock_init(clock,start): null descriptor -> ARGUMENT; invalid start -> VALUE;
  success records start without wall-clock access.
- cs_clock_observe(clock,sample): null -> ARGUMENT; invalid last -> STATE; invalid
  sample -> VALUE; sample before last -> BACKWARD. Equality is accepted. Success
  records the sample. No clamp hides a backward reading.
- cs_clock_advance(clock,duration): null -> ARGUMENT; invalid last -> STATE;
  invalid duration -> VALUE; overflow -> OVERFLOW. Success advances last by the
  exact duration, including zero. This is the deterministic test provider.

## Windows provider and viewer

The host provider caches QueryPerformanceFrequency and a nonnegative initial
QueryPerformanceCounter reading. V1 supports positive frequency <= UINT32_MAX;
unsupported frequency rejects explicitly. Reads use ticks since initialization,
floor to canonical seconds/nanoseconds, and reject negative, backward or overflow
samples. Store the previous raw counter as well as normalized time, so sub-unit
backward ticks are detected. Failed initialization/read preserves provider/output.

Pure conversion takes uint64_t elapsed ticks and uint32_t frequency. Null output
-> ARGUMENT; zero frequency -> FREQUENCY; seconds > UINT32_MAX -> OVERFLOW.
Remainder*1e9 fits uint64_t under the frequency cap; host-only integer division
and helpers do not enter the portable core. Windows result values are OK=0,
ARGUMENT=1, STATE=2, UNAVAILABLE=3, FREQUENCY=4, BACKWARD=5, OVERFLOW=6.
Raw initialization/failure transitions are independently injectable in tests,
and live QPC initialization/repeated reads are exercised without exact-delay claims.

Raw start validates null descriptor, availability, frequency, then nonnegative
counter. Raw sample validates descriptor/output, frequency/nonnegative ordered
origin/previous and canonical last (STATE), availability/nonnegative counter,
raw monotonicity, conversion overflow, then normalized monotonicity. Input/output
and provider must be disjoint; caller owns their truthful bounds/lifetime. Failure
preserves all fields and may retry. Only explicit successful start resets an epoch.

The viewer uses a 25ms Windows timer as a wakeup hint and checks our deadline
predicate against a fresh provider sample. An initial Space press switches scenes
and illuminates an original bottom-edge strip for 250ms; repeat/release does not
rearm it. A controllable fake source uses the same sampling/deadline/render path.
No duration is inferred from timer-message count. Delayed wakeups clear on the
next delivered sample; there is no promised scheduler/physical display accuracy.
Window destruction retires the timer; already-posted messages grant no lifetime.

## Acceptance and scope

Independent literal carry/borrow/deadline/error/reset traces plus uint64_t oracle
in host tests over boundary pairs, including maximum seconds. Whole field guards
check transactional errors. Test provider conversion with non-divisible frequencies,
frequency limits, large ticks, raw-backward rejection and failure/retry transitions;
test actual QPC reads. Hidden viewer fake deadlines immediately before/at/after,
delayed wakeup, repeat behavior, native timer lifecycle and rendered strip pixels.

Run existing x64 Debug twins, Release, actual i686 and validated x64 ASan/UBSan,
optimized core import/data audits and ARM64 LE/BE compile-only checks. No debugger
acceptance, B1 closure, m68k execution, historical compatibility or edition boot.
Owner's VirtualBox availability is a future native-PC test option; its firmware,
VM configuration and post-handoff device behavior need separate review/evidence.
