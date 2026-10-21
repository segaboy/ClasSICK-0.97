# SPEC-0058 - Explicit per-poll PC mouse delivery

2026-10-21;SRC-0338..0340;ADR-0066;IMPL-0061;TEST-0069.
Original engineering extension of SPEC-0040;no new hardware interface facts.

Keep cs_ps2_bus layout and cs_ps2_bus_poll behavior. Add
cs_ps2_bus_poll_delivery with the same arguments plus a required disjoint live
cs_ps2_bus_delivery:kind and raw packet,16 bytes on supported native ABIs.
Null bus/io/read/write/queue/delivery returns ARGUMENT without I/O or any
receipt/state mutation. Otherwise clear the receipt to NONE with zero payload
before processing,including terminal/state/clock failures. Return the same
CS_PS2 result as the legacy poll. No second poll/read/decoder or callback.

Only READY AUX handling emits an event. A complete valid report emits PACKET
and its unmodified raw deltas/buttons after existing left-button accounting
or unsynced rebaseline succeeds. Every accepted report emits once,including
identical reports and saturated diagnostic counters. Partial/idle/keyboard
traffic and startup ACK/BAT/ID/configuration emit NONE. Corrupt READY AUX,
bad tag or whole-report overflow emits LOST with zero payload after existing
decoder/counter cancellation. Corrupt non-AUX is keyboard-only and emits NONE.
An internal decoder/left-counter failure emits NONE and returns its existing
terminal result;no successful delivery is promised after that failure.

The value describes only this invocation. It is not a queue or replay token.
Caller consumes PACKET once through SPEC-0057;LOST once through the shared
owner's cancel service. Shared owner count is authoritative for that UI;do not
add the legacy bus counter. An owner fault stops further input until explicit
recovery/retirement. Unreported byte loss and hardware resynchronization remain
unprovable under the existing profile.

Tests cover authored full startup and READY traffic,I/O bounds,old API state/
queue/traffic equivalence,saturation,zero/stale receipts,arguments and terminal
failures. A baseline-source comparison separately uses the original public
bus source pinned before this change. End-to-end hosted tests feed receipt
events into the native window owner and compare authored logical samples and
full frames using the previously verified renderer. All are E1 synthetic
transcripts,not device/GUI/human evidence.

Existing native PC candidates include this changed controller source and must
be rebuilt/requalified before a new target claim. Released payloads/tags remain
immutable. No window boot binding,new device command,VM/emulator/hardware/
reference run or release. Mac M1-M7 and other compatibility gates persist.
