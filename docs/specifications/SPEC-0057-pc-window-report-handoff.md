# SPEC-0057 - PC mouse report to native window handoff

2026-10-21;SRC-0336/0337;ADR-0065;IMPL-0060;TEST-0068.
Original engineering adapter over SPEC-0039 and SPEC-0055. No historical
Macintosh or physical-device directional qualification is asserted.

`cs_pc_window_packet` accepts one complete decoded standard report and a live
shared window owner. Null inputs, dx/dy outside -256..255, or buttons outside
0..7 return CS_WINDOW_INPUT_ARGUMENT before effects. Validate before negating
Y, including INT32_MIN rejection. The chosen profile maps dx directly and dy
to -dy, preserves all three button bits, and calls cs_window_input_sample once.
Return its result unchanged, including fault/refusal and its retained effects.
No additional counter, decoder, state, allocation or automatic repair exists.
The adapter never clamps before the shared pointer owner: motion precedes the
button edge, including final motion on release. Saturated press counters do
not suppress delivery or window/cursor changes.

Caller owns single delivery and truthful disjoint owner/packet/borrowed objects.
Partial decoder input produces no owner call. A known corrupt, overflowed or
dropped report invokes cs_window_input_cancel once at the owning boundary;
preserve count/pattern, end dragging and require an observed LEFT release.
Neither packet nor error totals prove freshness: SPEC-0040 totals saturate.
There is no inferred recovery from unreported byte loss. A fault stops input
until explicit owner recovery/retirement under SPEC-0055.

The hosted test connects original raw bytes through the existing decoder to
this adapter, comparing against independently supplied logical samples and
explicit semantic expectations. This verifies the chosen transform, not its
direction on an actual device. Both monochrome and RGBA RAM frames are used;
whole-frame equivalence reuses the independently verified shared renderer.

This slice installs no controller poller or boot binding. Before a native PC
binding, the bus must expose a per-poll fresh packet/loss indication even when
its counters saturate; one controller owner must route startup and keyboard
traffic, and known loss must reach cancellation. Direct framebuffer mapping,
ordinary-RAM cursor storage, trace/lifecycle and actual memory costs also need
the binding contract. Target directional and GUI qualification remain separate.
Mac M1-M7 remain open. No emulator/VM/hardware/reference run or release.
