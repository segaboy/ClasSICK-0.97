# ADR-0065 - Keep PC report normalization at the native boundary

Accepted2026-10-21;SRC-0336/0337;SPEC-0057.

Use one stateless adapter for complete reports from the existing PS/2 decoder.
Validate its full numeric domain, invert raw Y under the declared engineering
profile, preserve button bits and delegate once to the shared window owner.
The owner retains edge ordering, clamping, counting, rendering and fault state.
Use explicit delivery, never a difference of saturating diagnostic counters.
Known loss routes through the existing cancellation service.

This small change is implemented directly by Codex: external briefing/review
would add comparable work. Existing worker qualifications are reused;no call
or quota saving is claimed. PC boot integration and Mac hardware-source closure
remain separately scoped. Old cores,drivers,workloads and releases are unchanged.
