# SPEC-0004 — Bounded normalized keyboard events

Status: finalized v1, 2026-10-07, project lead self-review under SRC-0021.
Independent core design contract; IMPL-0005 / TEST-0007. No historical Event
Manager, keyboard layout, text/IME, pointer, device driver or boot claim.

## Records and ownership

The first normalized keys are SPACE=1 and ESCAPE=2; actions PRESS=1 and RELEASE=2.
An input event contains four uint32_t fields: source, key, action and repeat.
Source is any nonzero caller-assigned stream identity, never a pointer; repeat
is 0 or 1, and must be zero on release. Events do not assert physical key state
or balanced press/release pairs. Unsupported keys/actions are rejected.

The caller owns a truthful live array of cs_input_record objects, suitably aligned
and effective-typed, capacity <= PTRDIFF_MAX / sizeof(cs_input_record). A record
contains an event and a uint32_t sequence. The separate queue descriptor contains
storage, capacity, head, count and next_sequence. A null array is allowed only
with zero capacity; head is zero for that queue, otherwise head < capacity;
count <= capacity. The descriptor, array and call input/output objects must be
disjoint. Callers keep them alive, synchronize access and never modify occupied
records or shallow-copy a live descriptor into another queue. Metadata checks
cannot prove physical bounds, alignment, ownership or lifetime.

There are no timestamps in v1. FIFO order is independent of clocks. Accepted
events receive sequences 1..UINT32_MAX, strictly increasing within a reset epoch.
After assigning UINT32_MAX, next_sequence becomes zero: further pushes reject
until reset. No wrap, gaps from rejected input, overwrite, silent drop, coalescing,
allocation, libc call or mutable global exists. Callers distinguish reset epochs
externally; sequence is not a persistent or globally unique identity.

## Operations and error precedence

Result values: OK=0, ARGUMENT=1, STATE=2, EVENT=3, FULL=4, EMPTY=5,
SEQUENCE=6, OVERFLOW=7.

- Init: null descriptor -> ARGUMENT; capacity beyond arithmetic limit -> OVERFLOW;
  null nonempty storage -> ARGUMENT. Success sets head/count=0,next_sequence=1,
  without reading or writing the backing array.
- Push: null descriptor/input -> ARGUMENT; invalid metadata -> STATE; invalid
  event -> EVENT; count==capacity -> FULL; next_sequence==0 -> SEQUENCE. Success
  copies event fields into the next free slot, assigns the sequence, increments
  count/sequence and wraps only the slot index. Source event is unchanged.
- Pop: null descriptor/output -> ARGUMENT; invalid metadata -> STATE; count==0
  -> EMPTY. Success copies the oldest record, advances head with slot wrap and
  decrements count. Removed slot bytes are preserved. Remaining order is unchanged.
- Reset: null descriptor -> ARGUMENT; invalid metadata -> STATE. Success discards
  all pending records, head/count=0,next_sequence=1. Backing bytes are preserved.

Every rejection preserves descriptor fields, output fields and backing bytes.
Operations do constant bounded work; integer checks precede array access. Capacity
zero is always full and empty. Independent queues must own disjoint arrays.

## Windows integration and acceptance

SRC-0022 supplies Microsoft interface prose only. Map WM_KEYDOWN/WM_KEYUP for
VK_SPACE/VK_ESCAPE to these logical keys, one record per delivered message.
Press repeat uses lParam bit 30; release repeat is zero. Repeat count, scan codes,
system keys, text and other messages are outside this adapter. Unsupported input
returns IGNORED without changing output; null output or zero source returns
ARGUMENT first.
This stateless mapping does not access host key state, time or the core queue.

The viewer reserves sixteen typed records from its host-owned arena, routes both
synthetic and mapped events through push/pop and one consumer, switches on an
initial Space press and closes on an initial Escape press. Release/repeat records
are consumed without a command. Queue overflow is surfaced as viewer failure,
never overwritten; the queue preserves pending events. Source 1 denotes the
viewer test producer and 2 the Windows adapter by application convention.

Test literal FIFO/wrap traces, whole-array guards, empty/full/retry/reset, invalid
metadata/events, sequence exhaustion, source preservation and independent queues.
An independent list oracle explores exhaustive short traces on tiny capacities.
Verify actual Win32 message dispatch and direct synthetic injection through the
same hidden viewer, plus adapter ignored messages and repeat mapping.
Run x64 Debug twins, Release, actual i686, validated ASan/UBSan, optimized core
import/data audits and ARM64 LE/BE compile-only checks. Clock/debugger, full input
and native drivers remain open; this does not finish B1/M0.1 or any boot edition.
